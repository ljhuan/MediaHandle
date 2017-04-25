namespace cpp Media

exception MyException {
    1: i32 whatOp,
    2: string why
}

service MediaServer {
    /**
     * login server, return token
     */
    string login(string ip),

    /**
     * acquire server ip
     */
    string getServerIP(),
    
	/**
     * shutdown server
     */
    void shutdownServer(),

    /**
     * create task, return taskid
     */
    string creatTask(string path),
     
    /**
     * delete task with taskid
     */ 
    void deleteTask(string taskid)
}
