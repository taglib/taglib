#include <iostream>

#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tbytevector.h>
#include <taglib/tfile.h>

using namespace std;

class imagefile : public TagLib::File
{
public:
    imagefile(const char *file) : TagLib::File(file) { }

    TagLib::ByteVector data()
    {
        return readBlock(length());
    }

private:
    virtual TagLib::Tag* tag() const
    {
        return 0;
    }
    virtual TagLib::AudioProperties* audioProperties() const
    {
        return 0;
    }
    virtual bool save()
    {
        return false;
    }
};

int main(int argc, char **argv)
{
    if (argc < 3) {
        cout<<"provide arguments in the following format:"<<endl;
        cout<<"update song path img path"<<endl;
        return -1;
    }

    const auto songpath = argv[1];
    const auto imgpath = argv[2];

    imagefile img(imgpath);
    TagLib::MPEG::File song(songpath);
    TagLib::ID3v2::Tag *tag = song.ID3v2Tag();

    auto framelist = tag->frameListMap()["APIC"];

    if (!framelist.isEmpty()) {
        for (auto it = tag->frameList().begin(); it != tag->frameList().end(); ++it) {
            auto frameID = (*it)->frameID();
            string framestr{frameID.data(), frameID.size()};

            if (framestr.compare("APIC") == 0) {
                cout<<"removing APIC frame"<<endl;
                tag->removeFrame((*it));
                it = tag->frameList().begin();
            }
        }
    } else {
        cout<<"song does not contain any cover art"<<endl;
    }

    auto picframe = new TagLib::ID3v2::AttachedPictureFrame;
    picframe->setPicture(img.data());
    picframe->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);

    tag->addFrame(picframe);

    song.save();

    cout<<"saved cover to the mp3 as the front cover"<<endl;

    return 0;
}
