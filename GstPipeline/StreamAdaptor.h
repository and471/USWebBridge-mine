#ifndef STREAMADAPTOR_H
#define STREAMADAPTOR_H

enum StreamProfile {
    NONE, _2D, _3D
};

class StreamAdaptor {
public:

    void setProfile(StreamProfile profile);

    int getFPS(int bitrate);

private:
    StreamProfile profile = NONE;
};

#endif // ULTRASOUNDSTREAMADAPTOR_H
