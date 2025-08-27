#include "MyRPCController.h"

MyRPCController::MyRPCController()
{
    m_failed = false;
    m_errText = "";
}

void MyRPCController::Reset()
{
    m_failed = false;
    m_errText = "";
}

bool MyRPCController::Failed() const
{
    return m_failed;
}

std::string MyRPCController::ErrorText() const
{
    return m_errText;
}

void MyRPCController::SetFailed(const std::string &reason)
{
    m_failed = true;
    m_errText = reason;
}

void MyRPCController::StartCancel() {}
bool MyRPCController::IsCanceled() const { return false; }
void MyRPCController::NotifyOnCancel(google::protobuf::Closure *callback) {}