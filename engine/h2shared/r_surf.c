/* r_surf.c -- surface-related refresh code
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "quakedef.h"
#include "r_local.h"

drawsurf_t	r_drawsurf;

ASM_LINKAGE_BEGIN
int		lightleft, sourcesstep, blocksize, sourcetstep;
int		lightdelta, lightdeltastep;
int		lightright, lightleftstep, lightrightstep, blockdivshift;
unsigned int	blockdivmask;
void		*prowdestbase;
unsigned char	*pbasesource;
int		surfrowbytes;	// used by ASM files
unsigned int	*r_lightptr;
int		r_stepback;
int		r_lightwidth;
int		r_numhblocks, r_numvblocks;
unsigned char	*r_source, *r_sourcemax;
ASM_LINKAGE_END

static unsigned int	blocklights[18*18];

#if !id386
static void R_DrawSurfaceBlock16 (void);
static void R_DrawSurfaceBlock8_mip0 (void);
static void R_DrawSurfaceBlock8_mip1 (void);
static void R_DrawSurfaceBlock8_mip2 (void);
static void R_DrawSurfaceBlock8_mip3 (void);
#endif

static void	(*surfmiptable[4])(void) =
{
	R_DrawSurfaceBlock8_mip0,
	R_DrawSurfaceBlock8_mip1,
	R_DrawSurfaceBlock8_mip2,
	R_DrawSurfaceBlock8_mip3
};

//=============================================================================


/*
===============
R_AddDynamicLights
===============
*/
static void R_AddDynamicLights (void)
{
	int			lnum;
	int			sd, td;
	float		dist, rad, minlight, lightval;
	vec3_t		impact;
	vec_t			local0, local1;
	int			s, t;
	int			i;
	int			smax, tmax;
	unsigned int	*pos;
	mtexinfo_t	*tex;
	msurface_t	*surf;

	if (!r_dynamic.integer)
		return;

	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum = 0; lnum < MAX_DLIGHTS; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		rad = fabs(cl_dlights[lnum].radius);
		dist = DotProduct (cl_dlights[lnum].origin, surf->plane->normal) - surf->plane->dist;
		rad -= fabs(dist);
		minlight = cl_dlights[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;

		if (cl_dlights[lnum].radius < 0)
			rad = -rad;

		for (i = 0; i < 3; i++)
		{
			impact[i] = cl_dlights[lnum].origin[i] - surf->plane->normal[i]*dist;
		}

		local0 = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local1 = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

		pos = blocklights;
		for (t = 0; t < tmax; t++)
		{
			td = local1 - t*16;
			if (td < 0)
				td = -td;
			for (s = 0; s < smax; s++, pos++)
			{
				sd = local0 - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)
				{
					unsigned int	temp;
					temp = (rad - dist)*256;
					i = t*smax + s;
					if (!cl_dlights[lnum].dark)
					{
						if (rad >= 0)
							*pos += temp;
						else
						{
							lightval = (dist + rad)*256;
							if (lightval < 0 && -lightval > blocklights[t*smax + s])
								*pos = 0;
							else
								*pos += lightval;
						}
					}
					else
					{
						if (*pos > temp)
							*pos -= temp;
						else
							*pos = 0;
					}
				}
			}
		}
	}
}

/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the 8.8 format in blocklights
===============
*/
static void R_BuildLightMap (void)
{
	int		smax, tmax;
	int		t;
	int		i, size;
	byte		*lightmap;
	unsigned int	scale;
	int		maps;
	msurface_t	*surf;
	int		light;

	surf = r_drawsurf.surf;

	smax = (surf->extents[0] >> 4) + 1;
	tmax = (surf->extents[1] >> 4) + 1;
	size = smax*tmax;
	lightmap = surf->samples;

	if (r_fullbright.integer || !cl.worldmodel->lightdata)
	{
		for (i = 0; i < size; i++)
			blocklights[i] = 0;
		return;
	}

	if ((currententity->drawflags & MLS_ABSLIGHT) == MLS_ABSLIGHT)
	{
		light = (255-currententity->abslight)<<VID_CBITS;
		for (i = 0; i < size; i++)
			blocklights[i] = light;	// 0 - 16383, 0 = full bright
		return;
	}

// clear to ambient
	for (i = 0; i < size; i++)
		blocklights[i] = r_refdef.ambientlight<<8;

// add all the lightmaps
	if (lightmap)
	{
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++)
		{
			scale = r_drawsurf.lightadj[maps];	// 8.8 fraction
			for (i = 0; i < size; i++)
				blocklights[i] += lightmap[i] * scale;
			lightmap += size;	// skip to next lightmap
		}
	}

// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights ();

// bound, invert, and shift
	for (i = 0; i < size; i++)
	{
		t = (255*256 - (int)blocklights[i]) >> (8 - VID_CBITS);

		if (t < (1 << 6))
			t = (1 << 6);

		blocklights[i] = t;
	}
}


/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
texture_t *R_TextureAnimation (texture_t *base)
{
	int		reletive;
	int		count;

	if (currententity->frame)
	{
		if (base->alternate_anims)
			base = base->alternate_anims;
	}

	if (!base->anim_total)
		return base;

	reletive = (int)(cl.time*10) % base->anim_total;

	count = 0;
	while (base->anim_min > reletive || base->anim_max <= reletive)
	{
		base = base->anim_next;
		if (!base)
			Sys_Error ("%s: broken cycle", __thisfunc__);
		if (++count > 100)
			Sys_Error ("%s: infinite cycle", __thisfunc__);
	}

	return base;
}


/*
===============
R_DrawSurface
===============
*/
void R_DrawSurface (void)
{
	unsigned char	*basetptr;
	int		smax, tmax, twidth;
	int		u;
	int		soffset, basetoffset, texwidth;
	int		horzblockstep;
	unsigned char	*pcolumndest;
	void		(*pblockdrawer)(void);
	texture_t	*mt;

// calculate the lightings
	R_BuildLightMap ();

	surfrowbytes = r_drawsurf.rowbytes;

	mt = r_drawsurf.texture;

	r_source = (byte *)mt + mt->offsets[r_drawsurf.surfmip];

// the fractional light values should range from 0 to (VID_GRADES - 1) << 16
// from a source range of 0 - 255

	texwidth = mt->width >> r_drawsurf.surfmip;

	blocksize = 16 >> r_drawsurf.surfmip;
	blockdivshift = 4 - r_drawsurf.surfmip;
	blockdivmask = (1 << blockdivshift) - 1;

	r_lightwidth = (r_drawsurf.surf->extents[0] >> 4) + 1;

	r_numhblocks = r_drawsurf.surfwidth >> blockdivshift;
	r_numvblocks = r_drawsurf.surfheight >> blockdivshift;

	if (r_pixbytes == 1)
	{
		pblockdrawer = surfmiptable[r_drawsurf.surfmip];
	// TODO: only needs to be set when there is a display settings change
		horzblockstep = blocksize;
	}
	else
	{
		pblockdrawer = R_DrawSurfaceBlock16;
	// TODO: only needs to be set when there is a display settings change
		horzblockstep = blocksize << 1;
	}

	smax = mt->width >> r_drawsurf.surfmip;
	twidth = texwidth;
	tmax = mt->height >> r_drawsurf.surfmip;
	sourcetstep = texwidth;
	r_stepback = tmax * twidth;

	r_sourcemax = r_source + (tmax * smax);

	soffset = r_drawsurf.surf->texturemins[0];
	basetoffset = r_drawsurf.surf->texturemins[1];

// << 16 components are to guarantee positive values for %
	soffset = ((soffset >> r_drawsurf.surfmip) + (smax << 16)) % smax;
	basetptr = &r_source[((((basetoffset >> r_drawsurf.surfmip)
					+ (tmax << 16)) % tmax) * twidth)];

	pcolumndest = r_drawsurf.surfdat;

	for (u = 0; u < r_numhblocks; u++)
	{
		r_lightptr = blocklights + u;

		prowdestbase = pcolumndest;

		pbasesource = basetptr + soffset;

		(*pblockdrawer)();

		soffset = soffset + blocksize;
		if (soffset >= smax)
			soffset = 0;

		pcolumndest += horzblockstep;
	}
}


//=============================================================================

#if	!id386
/*
================
R_DrawSurfaceBlock8_mip0
================
*/
static void R_DrawSurfaceBlock8_mip0 (void)
{
	int		v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (unsigned char *) prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 4;
		lightrightstep = (r_lightptr[1] - lightright) >> 4;

		for (i = 0; i < 16; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 4;

			light = lightright;

			for (b = 15; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)[(light & 0xFF00) + pix];
				light += lightstep;
			}

			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip1
================
*/
static void R_DrawSurfaceBlock8_mip1 (void)
{
	int		v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (unsigned char *) prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 3;
		lightrightstep = (r_lightptr[1] - lightright) >> 3;

		for (i = 0; i < 8; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 3;

			light = lightright;

			for (b = 7; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)[(light & 0xFF00) + pix];
				light += lightstep;
			}

			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip2
================
*/
static void R_DrawSurfaceBlock8_mip2 (void)
{
	int		v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (unsigned char *) prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 2;
		lightrightstep = (r_lightptr[1] - lightright) >> 2;

		for (i = 0; i < 4; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 2;

			light = lightright;

			for (b = 3; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)[(light & 0xFF00) + pix];
				light += lightstep;
			}

			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip3
================
*/
static void R_DrawSurfaceBlock8_mip3 (void)
{
	int		v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = (unsigned char *) prowdestbase;

	for (v = 0; v < r_numvblocks; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 1;
		lightrightstep = (r_lightptr[1] - lightright) >> 1;

		for (i = 0; i < 2; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 1;

			light = lightright;

			for (b = 1; b >= 0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)[(light & 0xFF00) + pix];
				light += lightstep;
			}

			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock16

FIXME: make this work
================
*/
static void R_DrawSurfaceBlock16 (void)
{
	int		k;
	int		lighttemp, lightstep, light;
	unsigned char	*psource;
	unsigned short	*prowdest;

	prowdest = (unsigned short *)prowdestbase;

	for (k = 0; k < blocksize; k++)
	{
		unsigned short	*pdest;
		unsigned char	pix;
		int			b;

		psource = pbasesource;
		lighttemp = lightright - lightleft;
		lightstep = lighttemp >> blockdivshift;

		light = lightleft;
		pdest = prowdest;

		for (b = 0; b < blocksize; b++)
		{
			pix = *psource;
			*pdest = vid.colormap16[(light & 0xFF00) + pix];
			psource += sourcesstep;
			pdest++;
			light += lightstep;
		}

		pbasesource += sourcetstep;
		lightright += lightrightstep;
		lightleft += lightleftstep;
		prowdest = (unsigned short *)((intptr_t)prowdest + surfrowbytes);
	}

	prowdestbase = prowdest;
}

#endif


//============================================================================

#if 0
/*
================
R_GenTurbTile
================
*/
static void R_GenTurbTile (pixel_t *pbasetex, void *pdest)
{
	int		*turb;
	int		i, j, s, t;
	byte	*pd;

	turb = sintable + ((int)(cl.time * SPEED) & (CYCLE-1));
	pd = (byte *)pdest;

	for (i = 0; i < TILE_SIZE; i++)
	{
		for (j = 0; j < TILE_SIZE; j++)
		{
			s = (((j << 16) + turb[i & (CYCLE-1)]) >> 16) & 63;
			t = (((i << 16) + turb[j & (CYCLE-1)]) >> 16) & 63;
			*pd++ = *(pbasetex + (t<<6) + s);
		}
	}
}


/*
================
R_GenTurbTile16
================
*/
static void R_GenTurbTile16 (pixel_t *pbasetex, void *pdest)
{
	int			*turb;
	int			i, j, s, t;
	unsigned short	*pd;

	turb = sintable + ((int)(cl.time * SPEED) & (CYCLE-1));
	pd = (unsigned short *)pdest;

	for (i = 0; i < TILE_SIZE; i++)
	{
		for (j = 0; j < TILE_SIZE; j++)
		{
			s = (((j << 16) + turb[i & (CYCLE-1)]) >> 16) & 63;
			t = (((i << 16) + turb[j & (CYCLE-1)]) >> 16) & 63;
			*pd++ = d_8to16table[*(pbasetex + (t<<6) + s)];
		}
	}
}


/*
================
R_GenTile
================
*/
void R_GenTile (msurface_t *psurf, void *pdest)
{
	if (psurf->flags & SURF_DRAWTURB)
	{
		if (r_pixbytes == 1)
		{
			R_GenTurbTile ((pixel_t *)
				((byte *)psurf->texinfo->texture + psurf->texinfo->texture->offsets[0]), pdest);
		}
		else
		{
			R_GenTurbTile16 ((pixel_t *)
				((byte *)psurf->texinfo->texture + psurf->texinfo->texture->offsets[0]), pdest);
		}
	}
	else if (psurf->flags & SURF_DRAWSKY)
	{
		if (r_pixbytes == 1)
		{
			R_GenSkyTile (pdest);
		}
		else
		{
			R_GenSkyTile16 (pdest);
		}
	}
	else
	{
		Sys_Error ("Unknown tile type");
	}
}
#endif

