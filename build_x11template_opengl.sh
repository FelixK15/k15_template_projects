C_FILE_TO_COMPILE="k15_x11template_opengl.c"
GCC_OPTIONS="-ansi -std=c99 -g3 -L/usr/lib -lX11 -lGL"
gcc $C_FILE_TO_COMPILE $GCC_OPTIONS