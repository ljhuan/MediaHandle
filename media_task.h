#ifndef MEDIA_TASK_H_
#define MEDIA_TASK_H_

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <fstream>
#include "media_element.h"
#include "media_process.h"
#include "media_type.h"
#include "jpeg-turbo/turbojpeg.h"
#include "boost/filesystem.hpp"
#include "media_algorithm.h"
#include "media_algorithm.h"
#include "db.h"
#include "result.hxx"
#include "result-odb.hxx"

// using namespace std;
using namespace boost::filesystem;
// usage
//void usage(const char *cmd) {
//    std::cout << "usage: " << cmd << " image-file-path" << std::endl;
//}

class MPGenerator : public BaseMediaProcessGenerator {
public:
    MPGenerator(const std::string &path) : path_(path) {
        it_ = recursive_directory_iterator(path_);
    }
    virtual const size_t getInputCount() const {
        return 0;
    }

    virtual const size_t getOutputCount() const {
        return 1;
    }
    virtual bool generate() {
        running_ = true;
        while (running_ && (itEnd_ != it_)) {
            auto element = loadPicture(it_->path());
            if (nullptr != element) {
                outputHandlers_[outputID_](element);
            }
            ++it_;
            return true;
        }

        return false;
    }

    virtual void interrupt() {
        running_ = false;
    }
private:
    bool running_ = false;
    uint32_t outputID_ = 0;
    path path_;
    recursive_directory_iterator it_;
    recursive_directory_iterator itEnd_;

    shared_ptr<BaseMediaElement> loadPicture(const boost::filesystem::path & filename) {
        if (is_regular_file(filename)) {
            auto size = file_size(filename);
            auto mdbuf = std::make_shared<BaseMediaBuffer>(size);
            std::ifstream ifs(filename.string(), (std::ios::in | std::ios::binary));
            ifs.read(reinterpret_cast<char *>(mdbuf->data()), size);
            ifs.close();
            auto element = std::make_shared<BaseMediaElement>();
            element->setMediaBuffer("jpg", mdbuf);
            element->setMetadata<uint32_t>("jpg_size", size);
            element->setMetadata<string>("name", filename.filename().string());
            element->setMetadata<string>("path", filename.string());
            element->setMetadata<uint32_t>("step", 1);
            return element;
        }
        return nullptr;
    }
};

class MPDecode : public BaseMediaProcessPipe {
public:
    MPDecode() {
        // create a decompress handle
        handle_ = tjInitDecompress();
    }
    virtual const size_t getInputCount() const {
        return 1;
    }

    virtual const size_t getOutputCount() const {
        return 1;
    }
    virtual void input(const size_t &index, const std::shared_ptr<BaseMediaElement> &mediaElement) {
        auto jpg = mediaElement->getMediaBuffer("jpg");
        auto jpgSize = mediaElement->getMetadata<uint32_t>("jpg_size");
        
        int32_t width = 0, height = 0, subsamp = 0;
        
        // redecode for width and heiht
        int32_t ret = tjDecompressHeader2(handle_, reinterpret_cast<uint8_t *>(jpg->data()), jpgSize, &width, &height, &subsamp);
        if (ret != 0) {
            cout << "pre decode error!\n";
        }
        
        // Decompress a JPEG image to a YUV planar image
        auto yuv = make_shared<Yuv420>((uint32_t)width, (uint32_t)height);
        tjDecompressToYUV(handle_, reinterpret_cast<uint8_t *>(jpg->data()), jpgSize, reinterpret_cast<uint8_t *>(yuv->getY()), TJFLAG_NOREALLOC);
        mediaElement->setMediaBuffer("yuv", yuv);

        mediaElement->setMetadata<uint32_t>("step", 2);
        outputHandlers_[index](mediaElement);
    }
    ~MPDecode() {
        tjDestroy(handle_);
    }
private:
    tjhandle handle_;
};

class MPSimpleProcess : public BaseMediaProcessPipe {
public:
    MPSimpleProcess() {}
    virtual const size_t getInputCount() const {
        return 1;
    }

    virtual const size_t getOutputCount() const {
        return 1;
    }
    virtual void input(const size_t &index, const std::shared_ptr<BaseMediaElement> &mediaElement) {
        mediaElement->setMetadata<uint32_t>("step", 4);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        outputHandlers_[index](mediaElement);
    }
};

class MPResultSend : public BaseMediaProcessPipe {
public:
    MPResultSend(const std::string &taskID) : taskID_(taskID) {}
    virtual const size_t getInputCount() const {
        return 1;
    }

    virtual const size_t getOutputCount() const {
        return 1;
    }

    virtual void input(const size_t &index, const std::shared_ptr<BaseMediaElement> &mediaElement) {
        auto filename = mediaElement->getMetadata<string>("name");
        auto carpalte = mediaElement->getMetadata<string>("plate");
        auto db = Singleton<MPDB>::instance()->sqliteDB();
        if (db == nullptr) {
            LOG(ERROR) << "db is null";
        }
        
        try {
            odb::transaction t(db->begin());
            // odb::schema_catalog::create_schema(*db);
            MediaResult r(taskID_, filename, carpalte);
            cout << "taskID:" << taskID_ << endl;;
            db->persist(r);
            t.commit();
        } catch (const std::exception& ex) {
            LOG(ERROR) << "db operation error!";
        }
        outputHandlers_[index](mediaElement);
    }
private:
    std::string taskID_;
};

class MPResultShow : public BaseMediaProcessCollapsar {
public:
    MPResultShow() {}
    virtual const size_t getInputCount() const {
        return 1;
    }

    virtual const size_t getOutputCount() const {
        return 0;
    }
    virtual void input(const size_t &index, const std::shared_ptr<BaseMediaElement> &mediaElement) {
        // mediaElement->setMetadata<uint32_t>("step", 3);
        auto yuv = std::static_pointer_cast<Yuv420>(mediaElement->getMediaBuffer("yuv"));
        auto width = yuv->getWidth();
        auto height = yuv->getHeight();
#if 0
        // test wether yuv data is ok
        std::string name = to_string(width) + "x" + to_string(height) + "_" + mediaElement->getMetadata<string>("name");
        auto pos = name.find(".jpg");
        name = name.substr(0, pos);
        name += ".yuv";
        cout << "name:" << name << endl;
        std::string path = "./" + name;
        ofstream ofs(path, ios::out | ios::binary);
        ofs.write(reinterpret_cast<char*>(yuv->getY()), yuv->getWidth() * yuv->getHeight());
        ofs.write(reinterpret_cast<char*>(yuv->getU()), yuv->getWidth() * yuv->getHeight() / 4);
        ofs.write(reinterpret_cast<char*>(yuv->getV()), yuv->getWidth() * yuv->getHeight() / 4);
        ofs.close();
#endif
        cout << "pic:" << mediaElement->getMetadata<string>("path") << std::endl;
        cout << "car plate:" << mediaElement->getMetadata<string>("plate") << std::endl;
        // cout << "step:" << mediaElement->getMetadata<uint32_t>("step") << std::endl;
    }
};

class Task001: public BaseMediaProcessRunloop {
 public:
    Task001(const std::string &path, const std::string &taskID): BaseMediaProcessRunloop(
        std::make_shared<MPGenerator>(path),
        std::make_shared<MPDecode>(),
        std::make_shared<MP_H3C_LPR>(),
        std::make_shared<MPSimpleProcess>(),
        std::make_shared<MPResultSend>(taskID),
        std::make_shared<MPResultShow>()
    ) {}
};

#endif  // MEDIA_TASK_H_
//int main(int argc, char const *argv[]) {
//    // gflag parse shell argument (AFT)
//    if (argc != 2) {
//        usage(argv[0]);
//        std::exit(1);
//    }
//    string path = argv[1];
//    Task001 task(path);
//    task.run();
//    return 0;
//}
