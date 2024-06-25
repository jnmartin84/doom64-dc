/* c_convert.c  */

/*-----------------------------------*/
/* Color Converter RGB2HSV & HSV2RGB */
/*-----------------------------------*/

#include "doomdef.h"

/*
===================
=
= LightGetHSV
= Set HSV values based on given RGB
=
===================
*/

int LightGetHSV(int r,int g,int b) // 800020BC
{
	int h_, s_, v_;
    int min;
    int max;
    float deltamin;
    float deltamax;
    float j;
    float x = 0;
    float xr;
    float xg;
    float xb;
    float sum = 0;

    max = MAXINT;

    if(r < max) {
        max = r;
    }
    if(g < max) {
        max = g;
    }
    if(b < max) {
        max = b;
    }

    min = MININT;

    if(r > min) {
        min = r;
    }
    if(g > min) {
        min = g;
    }
    if(b > min) {
        min = b;
    }

    deltamin = (float)((double)min / (double)255.0);
    deltamax = deltamin - (float)((double)max / (double)255.0);

    if((double)deltamin == 0.0) {
        j = 0.0;
    }
    else {
        j = deltamax / deltamin;
    }

    if((double)j != 0.0)
    {
        xr = (float)((double)r / (double)255.0);
        xg = (float)((double)g / (double)255.0);
        xb = (float)((double)b / (double)255.0);

        if(xr != deltamin)
        {
            if(xg != deltamin)
            {
                if(xb == deltamin)
                {
                    sum = ((deltamin - xg) / deltamax + 4.0) -
                          ((deltamin - xr) / deltamax);
                }
            }
            else
            {
                sum = ((deltamin - xr) / deltamax + 2.0) -
                      ((deltamin - xb) / deltamax);
            }
        }
        else
        {
            sum = ((deltamin - xb) / deltamax) -
                  ((deltamin - xg) / deltamax);
        }

        x = (sum * 60.0);

        if(x < 0.0)
        {
            x = (float)((double)x + (double)360.0);
        }
		
		while (x > 360.0) {
				x -= 360.0;
		}
    }
    else
    {
        j = 0.0;
    }

    h_ = (int)(((double)x / (double)360.0) * (double)255.0);

    s_ = (int)((double)j * (double)255.0);

    v_ = (int)((double)deltamin * (double)255.0);	
    return (((h_&0xff) << 16) | ((s_&0xff) << 8) | (v_&0xff));// & 0x00FFFFFF);
}

/*
===================
=
= LightGetRGB
= Set RGB values based on given HSV
=
===================
*/

int LightGetRGB(int h, int s, int v) // 8000248C
{
   int r,g,b;

    float x;
    float j;
    float i;
    float t;
    int table;
    float xr = 0;
    float xg = 0;
    float xb = 0;

    j = (float)(((double)h / (double)255.0) * (double)360.0);

    if((double)360.0 <= (double)j) {
        j = (float)((double)j - (double)360.0);
    }

    x = ((double)s / (double)255.0);
    i = ((double)v / (double)255.0);

    if (x != 0.0)
    {
        table = (int)(j / 60.0);
        if(table < 6)
        {
            t = (j / 60.0);
            switch(table) {
            case 0:
                xr = i;
                xg = ((1.0 - ((1.0 - (t - (float)table)) * x)) * i);
                xb = ((1.0 - x) * i);
                break;
            case 1:
                xr = ((1.0 - (x * (t - (float)table))) * i);
                xg = i;
                xb = ((1.0 - x) * i);
                break;
            case 2:
                xr = ((1.0 - x) * i);
                xg = i;
                xb = ((1.0 - ((1.0 - (t - (float)table)) * x)) * i);
                break;
            case 3:
                xr = ((1.0 - x) * i);
                xg = ((1.0 - (x * (t - (float)table))) * i);
                xb = i;
                break;
            case 4:
                xr = ((1.0 - ((1.0 - (t - (float)table)) * x)) * i);
                xg = ((1.0 - x) * i);
                xb = i;
                break;
            case 5:
                xr = i;
                xg = ((1.0 - x) * i);
                xb = ((1.0 - (x * (t - (float)table))) * i);
                break;
            }
        }
    }
    else
    {
        xr = xg = xb = i;
    }

    r = (int)((double)xr * (double)255.0);

	g = (int)((double)xg * (double)255.0);

	b = (int)((double)xb * (double)255.0);

    return (((r&0xff) << 16) | ((g&0xff) << 8) | (b&0xff));// & 0x00FFFFFF;
}
