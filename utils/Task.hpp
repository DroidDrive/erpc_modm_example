#pragma once
/// this wrapper is from the original rtos documentation: 
// https://www.freertos.org/FreeRTOS_Support_Forum_Archive/July_2010/freertos_Is_it_possible_create_freertos_task_in_c_3778071.html

/// another more lightweight wrapper approach can be found here:
// https://stackoverflow.com/questions/45831114/c-freertos-task-invalid-use-of-non-static-member-function

#include <modm/processing/rtos.hpp>


/// i edited the given tasks by adding another modm specific wrapper on top

class TaskBase {
public:
    TaskHandle_t handle_;
    ~TaskBase() {
        vTaskDelete(handle_);
    return;
  }
};
class Task : public TaskBase {
public:
    Task(char const*name, unsigned portBASE_TYPE priority, unsigned portSHORT stackDepth=configMINIMAL_STACK_SIZE) {
        xTaskCreate(&taskfun, (const char*)name, stackDepth, this, priority, &handle_);
    }
    virtual void task() = 0;
    static void taskfun(void* param) {
        static_cast<Task *>(param)->task();
        vTaskDelete(static_cast<Task *>(param)->handle_);
    }
};

class ModmTask : public Task {
public:
    ModmTask(char const*name, unsigned portBASE_TYPE priority, unsigned portSHORT stackDepth=configMINIMAL_STACK_SIZE) 
    : Task(name, priority, stackDepth){
        MODM_LOG_INFO << "[System *] Task '" << name << "' created" << modm::endl;
        updateStatus();
    }
    ~ModmTask(){
        MODM_LOG_INFO << "[System #] Task '" << status_.pcTaskName << "' removed" << modm::endl;
    }
    static void taskfun(void* param) {
    static_cast<Task *>(param)->task();
        vTaskDelete(static_cast<Task *>(param)->handle_);
    }
public:
    void sleep(TickType_t ticks){
        vTaskDelay(ticks);
    }
    void updateStatus()
    {
        /* Use the handle to obtain further information about the task. */
        vTaskGetInfo( 
            /* The handle of the task being queried. */
            this->handle_,
            /* The TaskStatus_t structure to complete with information on xTask. */
            &this->status_,
            /* Include the stack high water mark value in the
            TaskStatus_t structure. */
            pdTRUE,
            /* Include the task state in the TaskStatus_t structure. */
            eInvalid 
        );
    }
public:
    TaskStatus_t status_;
private:
};