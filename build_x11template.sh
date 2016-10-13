C_FILE_TO_COMPILE="k15_x11template.c"
GCC_OPTIONS="-ansi -std=c99 -g3 -L/usr/X11/lib -lX11"
gcc $C_FILE_TO_COMPILE $GCC_OPTIONS
