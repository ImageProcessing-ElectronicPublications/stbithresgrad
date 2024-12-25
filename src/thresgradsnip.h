/**

Grad (aka "Gradient Snip") threshold v1.2
By zvezdochiot <mykaralw@yandex.ru>
This is free and unencumbered software released into the public domain.

QUICK START

    #include ...
    #include ...
    #define THRESHOLD_GRADSNIP_IMPLEMENTATION
    #include "thresgradsnip.h"
    ...
    unsigned int width = 0, height = 0;
    unsigned int widthb = 0, heightb = 0;
    unsigned char components = 1,  componentsb = 1;
    float coef = 0.75f, delta = 0.0f;
    unsigned char bound_lower = 0, bound_upper = 255;
    int info = 1;

    unsigned char* image = stbi_load("foo.png", &width, &height, &components, 0);
    unsigned char* blur = stbi_load("foo_blur.png", &widthb, &heightb, &componentsb, 0);
    unsigned char* threshold_global = (unsigned char*)malloc(components * sizeof(unsigned char));

    if ((width == widthb) && (height == heightb) && (components == componentsb) && (threshold_global != NULL))
    {
        image_threshold_gradsnip(width, height, components, coef, delta, bound_lower, bound_upper, info, image, blur, threshold_global);
        stbi_write_png("foo.thresgrad.png", width, height, components, image, 0);
    }

VERSION HISTORY

1.2  2024-12-25  "head"    Header release.
1.1  2024-12-17  "regulator"    Add regulator: delta and bounds: lower and upper.
1.0  2024-12-16  "init"    Initial release.

**/

#ifndef THRESHOLD_GRADSNIP_H
#define THRESHOLD_GRADSNIP_H
#ifdef __cplusplus
    extern "C" {
#endif

float image_threshold_gradsnip_value(unsigned int width, unsigned int height, unsigned char components, unsigned char* image, unsigned char* blur, unsigned char* threshold_global);
float image_threshold_gradsnip_apply(unsigned int width, unsigned int height, unsigned char components, float coef, float delta, unsigned char bound_lower, unsigned char bound_upper, unsigned char* image, unsigned char* blur, unsigned char* threshold_global);
void image_threshold_gradsnip(unsigned int width, unsigned int height, unsigned char components, float coef, float delta, unsigned char bound_lower, unsigned char bound_upper, int info, unsigned char* image, unsigned char* blur, unsigned char* threshold_global);

#ifdef __cplusplus
    }
#endif
#endif  /* THRESHOLD_GRADSNIP_H */

#ifdef THRESHOLD_GRADSNIP_IMPLEMENTATION
#include <stdlib.h>
#include <math.h>

float image_threshold_gradsnip_value(unsigned int width, unsigned int height, unsigned char components, unsigned char* image, unsigned char* blur, unsigned char* threshold_global)
{
    float gradient = 0.0f;
    if ((image != NULL) && (blur != NULL) && (threshold_global != NULL))
    {
        for (unsigned char c = 0; c < components; c++)
        {
            size_t i = c;
            double sum_gi = 0.0, sum_g = 0.0;
            for (unsigned int y = 0; y < height; y++)
            {
                double sum_gil = 0.0, sum_gl = 0.0;
                for (unsigned int x = 0; x < width; x++)
                {
                    float s = image[i];
                    float b = blur[i];
                    float g = (s < b) ? (b - s) : (s - b);
                    sum_gl += g;
                    sum_gil += (g * s);
                    i += components;
                }
                sum_g += sum_gl;
                sum_gi += sum_gil;
            }
            float threshold = (sum_g > 0) ? (sum_gi / sum_g) : 127.5f;
            threshold_global[c] = (threshold < 0.0f) ? 0 : ((threshold < 255.0f) ? (unsigned char)(threshold + 0.5f) : 255);
            gradient += (sum_g / width / height);
        }
    }
    gradient /= components;

    return gradient;
}

float image_threshold_gradsnip_apply(unsigned int width, unsigned int height, unsigned char components, float coef, float delta, unsigned char bound_lower, unsigned char bound_upper, unsigned char* image, unsigned char* blur, unsigned char* threshold_global)
{
    float bwm = 0.0f;
    if ((image != NULL) && (blur != NULL) && (threshold_global != NULL))
    {
        size_t count_black = 0;
        for (unsigned char c = 0; c < components; c++)
        {
            size_t i = c;
            float tg = threshold_global[c];
            for (unsigned int y = 0; y < height; y++)
            {
                for (unsigned int x = 0; x < width; x++)
                {
                    float s = image[i];
                    float b = blur[i];
                    float t = b * coef + tg * (1.0f - coef) + delta;
                    unsigned char retval = 255;
                    if ((s < bound_lower) || ((s <= bound_upper) && (s < t)))
                    {
                        retval = 0;
                        count_black++;
                    }
                    image[i] = retval;
                    i += components;
                }
            }
        }
        bwm = (double) count_black / (width * height * components);
    }
    return bwm;
}

void image_threshold_gradsnip(unsigned int width, unsigned int height, unsigned char components, float coef, float delta, unsigned char bound_lower, unsigned char bound_upper, int info, unsigned char* image, unsigned char* blur, unsigned char* threshold_global)
{
    if (bound_upper < bound_lower)
    {
        unsigned char bound = bound_lower;
        bound_lower = bound_upper;
        bound_upper = bound;
    }

    float gradient = image_threshold_gradsnip_value(width, height, components, image, blur, threshold_global);
    float bwm = image_threshold_gradsnip_apply(width, height, components, coef, delta, bound_lower, bound_upper, image, blur, threshold_global);
    if (info > 0)
    {
        fprintf(stderr, "INFO: gradient %f\n", gradient);
        if ((image != NULL) && (blur != NULL) && (threshold_global != NULL))
        {
            for (unsigned char c = 0; c < components; c++)
            {
                fprintf(stderr, "INFO: component %d : threshold %d\n", c, threshold_global[c]);
            }
        }
        fprintf(stderr, "INFO: BW metric %f\n", bwm);
    }
}

#endif  /* THRESHOLD_GRADSNIP_IMPLEMENTATION */
