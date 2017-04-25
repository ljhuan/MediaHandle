#ifndef RESULT_HXX_
#define RESULT_HXX_
#include <string>
#include <odb/core.hxx>

class MediaResult {
public:
    MediaResult(const std::string &taskid, const std::string &filename, const std::string &carplate) :
        taskid_(taskid), filename_(filename), carplate_(carplate) {}
private:
    MediaResult() {}
    friend class odb::access;
    std::string taskid_;
    std::string filename_;
    std::string carplate_;

    // for auto id
    uint32_t id_;
};

#pragma db object(MediaResult)
#pragma db member(MediaResult::id_) id auto column("id")
#pragma db member(MediaResult::taskid_) column("taskid")
#pragma db member(MediaResult::filename_) column("filename")
#pragma db member(MediaResult::carplate_) column("carplate")

#endif  // RESULT_HXX_
