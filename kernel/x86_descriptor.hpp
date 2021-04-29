#pragma once

enum class DescriptorType {
  kUpper8Bytes = 0,
  kLDT = 2,
  kTSSAvailable = 9,
  kTSSBusy = 11,
  kCallGate = 12,
  kInterruptGate = 14,
  kTrapGAt = 15,

  kReadWrite = 2,
  kExecuteRead = 10,
};
