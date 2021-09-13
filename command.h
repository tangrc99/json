//
// Created by trc on 2021/9/12.
//

#ifndef TREE_COMMAND_H
#define TREE_COMMAND_H

enum CommandType {
    DOUBLE, PLUS
};

class CommandQueue;

class Command {
protected:
    friend CommandQueue;

    enum CommandStatus{
        EMPTY,TRUE,UNDO
    };
private:
    CommandStatus status;

    virtual void doSomething(int) {
        std::cout<<"使用一个未定义行为的函数";
    }

    //建立起与下一个 Command 对象的指针关系
    void setNext(Command *_ptr) {
        _next = _ptr;
        _ptr->_previous = this;
    }

protected:
    //假设状态由一个整型来表示，可以将这个作为桥接，链接到具体的实现上
    int value = 0;

    explicit Command(CommandStatus v = EMPTY) : status(v) {}
    explicit Command(int v) : value(v), status(TRUE) {}

public:
    //存储上一次和下一次的操作。
    Command *_previous = nullptr;
    Command *_next = nullptr;

};

//完成实际的功能
class CommandDouble : public Command {
public:
    void doSomething(int i) override {
        value = i * 2 ;
    }

    CommandDouble():Command(TRUE){}

    //不允许给具体命令类进行初值化，因为他的值依赖于上一个命令。
    explicit CommandDouble(int v) = delete;
};

class CommandPlus : public Command {
public:
    void doSomething(int i) override {
        value = i + 1;
    }
    CommandPlus():Command(TRUE){}
    explicit CommandPlus(int v) = delete;

};


//这算是一个 builder 模式吗？
class CommandQueue {
private:
    Command* _pos;                          //指向当前最后一个Command对象
    int queue_length;                       //允许存在的最大命令数

public:
    //借助一个 vector 来实现一个循环队列
    explicit CommandQueue(const int &first_value, int length) : queue_length(length) {
        std::vector<Command *> queue;           //一个临时变量借助其来创建循环队列,不要调用
        queue.push_back(new Command(first_value));
        _pos = queue[0];    //创建的变量在栈区，并不会被回收。
        while (--length) {
            queue.push_back(new Command);
            //最后一个元素会指向他自身，暂时不管
            queue[queue_length - length - 1]->setNext(queue.back());
        }
        queue.back()->setNext(queue[0]);
    }

    ~CommandQueue(){
        //被覆盖的 Command 对象已经被释放，只需要释放最后队列中的
        while(queue_length--){
            Command *_tmp=_pos->_next;
            delete _pos;
            _pos=_tmp;
        }
    }

    void NewCommand(const CommandType &mode) {
        Command *_tmp = getCommandMode(mode);
        _tmp->setNext(_pos->_next->_next);
        delete _pos->_next;
        _pos->setNext(_tmp);
        _tmp->doSomething(_pos->value);
        _pos = _pos->_next;
    }

    static Command *getCommandMode(const int &mode) {
        if (mode == DOUBLE)
            return new CommandDouble;
        return new CommandPlus;
    }

    //用户接口，获得最后命令执行后的值
    int getvalue(){
        return _pos->value;
    }

    //撤销一个命令
    void undo(){
        if(_pos->_previous->status==Command::CommandStatus::TRUE){
            _pos->status=Command::CommandStatus::UNDO;
            _pos=_pos->_previous;
        }else{
            std::cout<<"没有上一条命令，无法撤销。"<<std::endl;
        }
    }

    //重做一个命令
    void redo(){
        if(_pos->_next->status==Command::CommandStatus::UNDO)
            _pos=_pos->_next;
        else{
            std::cout<<"没有被撤销的命令，无法重做。"<<std::endl;
        }
    }
};

int main(){
    std::shared_ptr<CommandQueue> command_queue(new CommandQueue(1,3));
    command_queue->undo();
    command_queue->NewCommand(DOUBLE);
    command_queue->NewCommand(DOUBLE);
    command_queue->NewCommand(DOUBLE);
    std::cout<<command_queue->getvalue()<<std::endl;
    command_queue->undo();
    std::cout<<command_queue->getvalue()<<std::endl;
    command_queue->redo();
    std::cout<<command_queue->getvalue()<<std::endl;
    command_queue->redo();

    return 0;
}

#endif //TREE_COMMAND_H
