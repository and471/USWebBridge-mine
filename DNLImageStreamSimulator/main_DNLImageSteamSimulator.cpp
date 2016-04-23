#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <cstdio>
#include <iostream>
#include <string>

#include <vtkSmartPointer.h>

#include "DNLImageSource.h"
#include "DNLFileImageSource.h"
#include "DNLFrameExchange.h"
#include "DNLImageExtractor.h"
#include <Modules/USStreamingCommon/DNLImage.h>
#include <vtkJPEGWriter.h>

void imaging_handler(DNLImage::Pointer imag);

static GMainLoop *loop;
static DNLFrameExchange* exchange = new DNLFrameExchange();

static void
cb_need_data (GstAppSrc *appsrc,
          guint       unused_size,
          gpointer    user_data)
{
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  GstFlowReturn ret;

  char* d;
  size_t s;
  DNLImageExtractor::get_jpeg(exchange->get_frame(), &d, &s);

  buffer = gst_buffer_new_allocate (NULL, s, NULL);
  gst_buffer_fill(buffer, 0, (guchar*)d, s);

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 20);
  timestamp += GST_BUFFER_DURATION (buffer);

  ret = gst_app_src_push_buffer(appsrc, buffer);

  if (ret != GST_FLOW_OK) {
    fprintf(stderr, "Gstreamer Error\n");
    g_main_loop_quit (loop);
  }
}

int main(int argc, char *argv[])
{

    if (argc < 2){
        std::cout << "Not enough arguments"<<std::endl;
        std::cout << "Usage:"<<std::endl;
        std::cout << argv[0]<<" "<< "<folder>"<<std::endl;
        return -1;
    }

    std::string folder = argv[1];

    DNLImageSource *dnlIS = new DNLFileImageSource(folder);
    dnlIS->connect(&imaging_handler); /// This will start producing signals
    //boost::bind(&MyOtherClass::MyFunction, boost::ref(myobject), _1) // Use this above if your function is in other class

    dnlIS->start();

    GstElement *pipeline, *appsrc, *jpegdec, *conv, *payloader, *udpsink, *videoenc;

        /* init GStreamer */
        gst_init (&argc, &argv);
        loop = g_main_loop_new (NULL, FALSE);

        /* setup pipeline */
        pipeline = gst_pipeline_new ("pipeline");
        appsrc = gst_element_factory_make ("appsrc", "source");
        jpegdec = gst_element_factory_make ("jpegdec", "j");
        conv = gst_element_factory_make ("videoconvert", "conv");
        videoenc = gst_element_factory_make("avenc_mpeg4", "ffenc_mpeg4");
        payloader = gst_element_factory_make("rtpmp4vpay", "rtpmp4vpay");
        g_object_set(G_OBJECT(payloader),
                "config-interval", 5,
                NULL);
        udpsink = gst_element_factory_make("udpsink", "udpsink");
        g_object_set(G_OBJECT(udpsink),
                "host", "127.0.0.1",
                "port", 5000,
                NULL);

        g_object_set (G_OBJECT (appsrc), "caps",
             gst_caps_from_string("image/jpeg"), NULL);

        // WORKS!
        gst_bin_add_many (GST_BIN (pipeline), appsrc, jpegdec, conv, videoenc, payloader, udpsink, NULL);
        gst_element_link_many (appsrc, jpegdec, conv, videoenc, payloader, udpsink, NULL);

        /* setup appsrc */
        g_object_set (G_OBJECT (appsrc),
                "stream-type", 0,
                "is-live", TRUE,
                "format", GST_FORMAT_TIME, NULL);
        GstAppSrcCallbacks cbs;
        cbs.need_data = cb_need_data;
        //cbs.enough_data = cb_enough_data;
        //cbs.seek_data = cb_seek_data;
        //g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), NULL);
        gst_app_src_set_callbacks(GST_APP_SRC_CAST(appsrc), &cbs, NULL, NULL);

        /* play */
        gst_element_set_state (pipeline, GST_STATE_PLAYING);
        g_main_loop_run (loop);

        /* clean up */
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (pipeline));
        g_main_loop_unref (loop);


        dnlIS->stop();
        delete dnlIS;


        return 0;
    }


/**
 * Here I just take the VTK image within the DNL image and print the spacing
 * as an example.
 */
void imaging_handler(DNLImage::Pointer image){
    exchange->add_frame(image);
}
