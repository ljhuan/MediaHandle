#ifndef TASK_HXX_
#define TASK_HXX_
#include <string>
#include "odb/core.hxx"  // {1}

class MediaTask {
public:
    MediaTask(const std::string &taskid, const std::string &imgPath) :taskid_(taskid), imgPath_(imgPath) {}
private:
    MediaTask() {}  // {2}
    friend class odb::access;  // {3}

    std::string taskid_;
    std::string imgPath_;
};

#pragma db object(MediaTask)
#pragma db member(MediaTask::taskid_) id column("id")
#pragma db member(MediaTask::imgPath_) column("path")

#endif  // TASK_HXX_
