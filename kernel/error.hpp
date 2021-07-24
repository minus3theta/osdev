#pragma once

#include <array>
#include <cstdio>

class Error {
public:
  enum Code {
    kSuccess,
    kFull,
    kEmpty,
    kNoEnoughMemory,
    kIndexOutOfRange,
    kHostControllerNotHalted,
    kInvalidSlotID,
    kPortNotConnected,
    kInvalidEndpointNumber,
    kTransferRingNotSet,
    kAlreadyAllocated,
    kNotImplemented,
    kInvalidDescriptor,
    kBufferTooSmall,
    kUnknownDevice,
    kNoCorrespondingSetupStage,
    kTransferFailed,
    kInvalidPhase,
    kUnknownXHCISpeedID,
    kNoWaiter,
    kNoPCIMSI,
    kUnknownPixelFormat,
    kNoSuchTask,
    kInvalidFormat,
    kLastOfCode,
  };

private:
  static constexpr std::array code_names = {
      "kSuccess",
      "kFull",
      "kEmpty",
      "kNoEnoughMemory",
      "kIndexOutOfRange",
      "kHostControllerNotHalted",
      "kInvalidSlotID",
      "kPortNotConnected",
      "kInvalidEndpointNumber",
      "kTransferRingNotSet",
      "kAlreadyAllocated",
      "kNotImplemented",
      "kInvalidDescriptor",
      "kBufferTooSmall",
      "kUnknownDevice",
      "kNoCorrespondingSetupStage",
      "kTransferFailed",
      "kInvalidPhase",
      "kUnknownXHCISpeedID",
      "kNoWaiter",
      "kNoPCIMSI",
      "kNoSuchTask",
      "kUnknownPixelFormat",
      "kInvalidFormat",
  };
  static_assert(Error::Code::kLastOfCode == code_names.size());

public:
  Error(Code code, const char *file, int line)
      : code(code), line(line), file(file) {}

  Code Cause() const { return code; }

  operator bool() const { return code != kSuccess; }

  const char *Name() const { return code_names[static_cast<int>(code)]; }

  const char *File() const { return file; }

  int Line() const { return line; }

private:
  Code code;
  int line;
  const char *file;
};

#define MAKE_ERROR(code) Error((code), __FILE__, __LINE__)

template <class T> struct WithError {
  T value;
  Error error;
};
