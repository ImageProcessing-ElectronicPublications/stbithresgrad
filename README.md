# stbithresgrad

Grad (aka Gradient Snip) threshold an image based of Gauss blur.
Used:
* Gauss blur: [iir_gauss_blur](https://github.com/arkanis/iir_gauss_blur).
* STB: [stb](https://github.com/nothings/stb).

This filter was first applied in [STEX](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental) (2024)
in a single-component version (Y).

Here this filter is implemented in a full-color version.

## Usage

`./stbithresgrad [-h] [-i] [-s sigma] [-k coeff] input-file output.png`

`-s sigma`     The sigma of the gauss normal distribution (number >= 0.5).
               Larger values result in a stronger blur.

`-k coeff`     The coefficient local threshold.

`-i`           Info to `stdout`.

`-h`           display this help and exit.

You can use either sigma to specify the strengh of the blur.

The performance is independent of the blur strengh (sigma). This tool is an
implementation of the paper "Recursive implementaion of the Gaussian filter"
by Ian T. Young and Lucas J. van Vliet.

stb_image and stb_image_write by Sean Barrett and others is used to read and
write images.

## Installation

- Clone the repo or download the source `git clone --recurse-submodules https://github.com/ImageProcessing-ElectronicPublications/stbithresgrad`
- Execute `make`
- Done. Either use the `stbithresgrad` executable directly or copy it somewhere in your PATH.

## Links

* STB: [stb](https://github.com/nothings/stb).
* Gauss blur: [iir_gauss_blur](https://github.com/arkanis/iir_gauss_blur).
