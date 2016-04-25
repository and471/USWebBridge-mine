#include <cstdio>
#include <gst/gst.h>
#include <string>
#include <glib.h>

#include <USPipeline/DNLImageSource.h>
#include <USPipeline/DNLFileImageSource.h>
#include <USPipeline/UltrasoundImagePipeline.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Not enough arguments\n");
        printf("Usage:\n");
        printf("%s <folder>\n", argv[0]);
        return -1;
    }

    gst_init(&argc, &argv);
    std::string folder = argv[1];

    GMainLoop* loop = g_main_loop_new(NULL, FALSE);

    DNLImageSource* dnl_image_source = new DNLFileImageSource(folder);
    UltrasoundImagePipeline *pipeline = new UltrasoundImagePipeline(NULL);
    pipeline->setDNLImageSource(dnl_image_source);

    pipeline->start();
    g_main_loop_run(loop);

    pipeline->stop();
    g_main_loop_unref(loop);

    return 0;
}
