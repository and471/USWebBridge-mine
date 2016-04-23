#ifndef ULTRASOUNDIMAGEPIPELINE_H
#define ULTRASOUNDIMAGEPIPELINE_H


class UltrasoundImagePipeline
{
public:
    UltrasoundImagePipeline();

signals:

public slots:

private:
    GstElement *pipeline, *appsrc, *jpegdec, *conv, *payloader, *udpsink, *videoenc;
};

#endif // ULTRASOUNDIMAGEPIPELINE_H
