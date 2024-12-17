#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define IIR_GAUSS_BLUR_IMPLEMENTATION
#include "iir_gauss_blur.h"

void usage(char* progname)
{
    fprintf(stderr,
        "%s %s %s\n",
        "Usage:", progname, "[-h] [-i] [-s sigma] [-k coeff] input-file output.png\n"
        "Grad (aka Gradient Snip) threshold an image and save it as PNG.\n"
     );
}

void help(float sigma, float coef)
{
    fprintf(stderr,
        "%s %f %s %f %s\n",
        "  -s sigma     The sigma of the gauss normal distribution (number >= 0.5, default =", sigma, ").\n"
        "               Larger values result in a stronger blur.\n"
        "  -k coeff     The coefficient local threshold (number, default =", coef, ").\n"
        "  -i           info to stdout.\n"
        "  -h           display this help and exit.\n"
        "\n"
        "You can use either sigma to specify the strengh of the blur.\n"
        "\n"
        "The performance is independent of the blur strengh (sigma). This tool is an\n"
        "implementation of the paper \"Recursive implementaion of the Gaussian filter\"\n"
        "by Ian T. Young and Lucas J. van Vliet.\n"
        "\n"
        "stb_image and stb_image_write by Sean Barrett and others is used to read and\n"
        "write images.\n"
    );
}

uint8_t* image_copy(unsigned int width, unsigned int height, unsigned char components, unsigned char* image)
{
    size_t image_size = height * width * components;
    uint8_t* dest = (unsigned char*)malloc(image_size * sizeof(unsigned char));
    if (dest != NULL)
    {
        for (size_t i = 0; i < image_size; i++)
        {
            dest[i] = image[i];
        }
    }
    return dest;
}

float image_threshold_grad_value(unsigned int width, unsigned int height, unsigned char components, unsigned char* image, unsigned char* blur, unsigned char* threshold_global)
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
            threshold_global[c] = (threshold < 0.0f) ? 0 : ((threshold < 255.0f) ? (uint8_t)(threshold + 0.5f) : 255);
            gradient += (sum_g / width / height);
        }
    }
    gradient /= components;

    return gradient;
}

float image_threshold_grad_apply(unsigned int width, unsigned int height, unsigned char components, float coef, unsigned char* image, unsigned char* blur, unsigned char* threshold_global)
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
                    float t = b * coef + tg * (1.0f - coef);
                    unsigned char retval = 255;
                    if (s < t)
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

int main(int argc, char** argv)
{
    int info = 0;
    float sigma = 10.0f;
    float coef = 0.75f;

    int opt;
    while ( (opt = getopt(argc, argv, "is:k:h")) != -1 )
    {
        switch(opt)
        {
            case 'i':
                info = 1;
                break;
            case 's':
                sigma = strtof(optarg, NULL);
                break;
            case 'k':
                coef = strtof(optarg, NULL);
                break;
            case 'h':
                usage(argv[0]);
                help(sigma, coef);
                return 0;
            default:
                usage(argv[0]);
                return 1;
        }
    }

    // Need at least two filenames after the last option
    if (argc < optind + 2)
    {
        usage(argv[0]);
        return 1;
    }

    int width = 0, height = 0, components = 1;
    uint8_t* image = stbi_load(argv[optind], &width, &height, &components, 0);
    if (image == NULL)
    {
        fprintf(stderr, "Failed to load %s: %s.\n", argv[optind], stbi_failure_reason());
        return 2;
    }

    uint8_t* blur = image_copy(width, height, components, image);
    if (blur == NULL)
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 3;
    }

    uint8_t* threshold_global = (unsigned char*)malloc(components * sizeof(unsigned char));;
    if (threshold_global == NULL)
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 3;
    }

    iir_gauss_blur(width, height, components, blur, sigma);

    float gradient = image_threshold_grad_value(width, height, components, image, blur, threshold_global);
    float bwm = image_threshold_grad_apply(width, height, components, coef, image, blur, threshold_global);
    if (info > 0)
    {
        fprintf(stderr, "INFO: image %s\n", argv[optind]);
        fprintf(stderr, "INFO: width %d\n", width);
        fprintf(stderr, "INFO: height %d\n", height);
        fprintf(stderr, "INFO: components %d\n", components);
        fprintf(stderr, "INFO: sigma %f\n", sigma);
        fprintf(stderr, "INFO: gradient %f\n", gradient);
        if ((image != NULL) && (blur != NULL) && (threshold_global != NULL))
        {
            for (unsigned char c = 0; c < components; c++)
            {
                fprintf(stderr, "INFO: component %d : threshold %d\n", c, threshold_global[c]);
            }
        }
        fprintf(stderr, "INFO: coeff. %f\n", coef);
        fprintf(stderr, "INFO: BW metric %f\n", bwm);
    }

    if ( stbi_write_png(argv[optind+1], width, height, components, image, 0) == 0 )
    {
        fprintf(stderr, "Failed to save %s.\n", argv[optind+1]);
        return 4;
    }

    return 0;
}
