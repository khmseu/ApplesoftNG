#pragma once

#include "core/applesoft_variables.hpp"
#include "core/io_ports.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace applesoft::asm_port {

template <typename T> class ApplesoftDualPointer {
  static_assert(
      std::is_trivially_copyable_v<T>,
      "ApplesoftDualPointer<T> requires a trivially copyable data type");

public:
  static ApplesoftDualPointer emulated(std::uint16_t address) {
    return ApplesoftDualPointer(Kind::Emulated, address, nullptr);
  }

  static ApplesoftDualPointer native(T *ptr) {
    return ApplesoftDualPointer(Kind::Native, 0u, ptr);
  }

  bool isEmulated() const noexcept { return kind_ == Kind::Emulated; }

  bool isNative() const noexcept { return kind_ == Kind::Native; }

  T &reference() const { return *nativePointer(); }

  T *nativePointer() const {
    if (kind_ == Kind::Emulated) {
      return emulatedTypedPointerOrThrow(emulatedAddress_);
    }

    if (pointer_ == nullptr) {
      throw std::runtime_error("ApplesoftDualPointer: native pointer is null");
    }

    return pointer_;
  }

  std::uint16_t emulatedPointerOrThrow() const {
    if (!isEmulated()) {
      throw std::runtime_error(
          "ApplesoftDualPointer: value does not hold an emulated pointer");
    }
    return emulatedAddress_;
  }

private:
  enum class Kind : std::uint8_t { Emulated, Native };

  ApplesoftDualPointer(Kind kind, std::uint16_t emulatedAddress, T *ptr)
      : kind_(kind), emulatedAddress_(emulatedAddress), pointer_(nullptr) {
    if (kind_ == Kind::Native) {
      pointer_ = ptr;
    }
  }

  static T *emulatedTypedPointerOrThrow(std::uint16_t address) {
    // Resolve through ApplesoftVariables pointer helpers to keep emulated
    // addressing behavior centralized.
    const std::uint16_t resolvedAddress =
        variables().pointer(address).address();

    if (resolvedAddress < 0x0100u) {
      throw std::runtime_error(
          "ApplesoftDualPointer: emulated address is in zero page mapped "
          "state and has no stable typed storage");
    }

    if (resolvedAddress >= ApplesoftVariables::ADDR_AS_INPUT_BUFFER_MINUS_4 &&
        resolvedAddress <= ApplesoftVariables::ADDR_AS_INPUT_BUFFER_MINUS_1) {
      throw std::runtime_error(
          "ApplesoftDualPointer: emulated address is in input-buffer "
          "sentinel mapped state and has no stable typed storage");
    }

    if (resolvedAddress >= ApplesoftVariables::ADDR_AS_INPUT_BUFFER &&
        resolvedAddress <=
            (ApplesoftVariables::ADDR_AS_INPUT_BUFFER + 0x00ffu)) {
      throw std::runtime_error(
          "ApplesoftDualPointer: emulated address is in input-buffer page "
          "mapped state and has no stable typed storage");
    }

    if (resolvedAddress >= IOPorts::ADDR_BASE &&
        resolvedAddress < IOPorts::ADDR_END) {
      throw std::runtime_error(
          "ApplesoftDualPointer: emulated address is in I/O mapped state and "
          "has no stable typed storage");
    }

    auto &vars = variables();
    const std::size_t start = static_cast<std::size_t>(resolvedAddress);
    if (start + sizeof(T) > vars.general_memory_.size()) {
      throw std::runtime_error(
          "ApplesoftDualPointer: emulated typed pointer range exceeds memory");
    }

    auto *raw = vars.general_memory_.data() + start;
    if ((reinterpret_cast<std::uintptr_t>(raw) % alignof(T)) != 0u) {
      throw std::runtime_error(
          "ApplesoftDualPointer: emulated typed pointer has incompatible "
          "alignment");
    }

    return reinterpret_cast<T *>(raw);
  }

  Kind kind_ = Kind::Native;
  std::uint16_t emulatedAddress_ = 0;
  T *pointer_ = nullptr;
};

} // namespace applesoft::asm_port
