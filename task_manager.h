#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <gflags/gflags.h>
#include "task.hxx"
#include "task-odb.hxx"
#include "singleton.h"
#include "media_task.h"
#include "log.h"
#include "db.h"
#include "taskinfo.pb.h"
#include "type.h"

DECLARE_bool(debug);
DECLARE_string(imgpath);

class TaskManager {
public:
    TaskManager() {}
    ~TaskManager() {}

    void createTask(const std::string &info, std::string &taskID) {
        TaskInfo tskinfo;
        if (!tskinfo.ParseFromString(info)) {
            LOG(ERROR) << "task info parse error";
            return;
        }

        taskID = std::to_string(taskCount_);
        ++taskCount_;
        std::string path;
        if (FLAGS(debug)) {
            path = FLAGS(imgpath);
        } else {
            path = tskinfo.imgpath();
        }

        auto task = std::make_shared<Task001>(path, taskID);
        {
            std::unique_lock<std::mutex> tlock(tasksMutex_);
            tasks_[taskID] = task;
        }

        // save task in db
        auto db = Singleton<MPDB>::instance()->sqliteDB();
        odb::transaction t(db->begin());
        // odb::schema_catalog::create_schema(*db);
        MediaTask tsk(taskID, path);
        db->persist(tsk);
        t.commit();

        LOG(INFO) << "createTask";
    }
    
    void deleteTask(const std::string &taskID) {
        // delete task in db
        auto db = Singleton<MPDB>::instance()->sqliteDB();
        odb::transaction t(db->begin());
        db->erase<MediaTask>(taskID);
        t.commit();

        auto it = tasks_.find(taskID);
        if (it != tasks_.end()) {
            std::unique_lock<std::mutex> tlock(tasksMutex_);
            tasks_.erase(taskID);
        } else {
            std::string error = "no such task, taskID: " + taskID;
            throw std::runtime_error(error);
        }
        LOG(INFO) << "deleteTask";
    }

    void startTask(const std::string &taskID) {
        auto it = tasks_.find(taskID);
        if (it != tasks_.end()) {
            // auto func = [this](std::string &taskID)->void { return tasks_[taskID]->run(); };
            std::thread *t = new std::thread(std::bind(&TaskManager::threadProc, this, taskID));
            std::unique_lock<std::mutex> thlock(threadsMutex_);
            threads_[taskID] = t;
        } else {
            std::string error = "no such task, taskID: " + taskID;
            LOG(ERROR) << error;
            throw std::runtime_error(error);
        }
        LOG(INFO) << "startTask";
    }

    void stopTask(const std::string &taskID) {
        auto it = tasks_.find(taskID);
        if (it != tasks_.end()) {
            tasks_[taskID]->stop();
            threads_[taskID]->join();
            delete threads_[taskID];
            std::unique_lock<std::mutex> thlock(threadsMutex_);
            threads_.erase(taskID);
        } else {
            std::string error = "no such task, taskID: " + taskID;
            throw std::runtime_error(error);
        }
        LOG(INFO) << "stopTask";
    }

    void stopAll() {
        for (auto task : tasks_) {
            stopTask(task.first);
        }
    }

private:
    void threadProc(std::string &taskID) {
        tasks_[taskID]->run();
    }
    std::mutex tasksMutex_;
    std::mutex threadsMutex_;
    uint32_t taskCount_ = 0;
    std::map<std::string, std::shared_ptr<BaseMediaProcessRunloop> > tasks_;
    std::map<std::string, std::thread*> threads_;
    SINGLETON_DECL(TaskManager);
};

#endif  // TASK_MANAGER_H_
