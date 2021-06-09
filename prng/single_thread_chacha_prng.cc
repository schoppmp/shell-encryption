/*
 * Copyright 2019 Google LLC.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "prng/single_thread_chacha_prng.h"

#include <vector>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "status_macros.h"

namespace rlwe {

SingleThreadChaChaPrng::SingleThreadChaChaPrng(absl::string_view in_key,
                                               int position_in_buffer,
                                               int salt_counter,
                                               std::vector<Uint8> buffer)
    : key_(in_key),
      position_in_buffer_(position_in_buffer),
      salt_counter_(salt_counter),
      buffer_(std::move(buffer)) {}

rlwe::StatusOr<std::unique_ptr<SingleThreadChaChaPrng>>
SingleThreadChaChaPrng::Create(absl::string_view in_key) {
  if (static_cast<int>(in_key.length()) != SeedLength()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Cannot create Prng with key of the "
                     "wrong size. Real ",
                     "key length of ", in_key.length(), " instead of expected ",
                     "key length of ", SeedLength(), "."));
  }
  int position_in_buffer = 0;
  int salt_counter = 0;
  std::vector<Uint8> buffer;
  RLWE_RETURN_IF_ERROR(
      internal::ChaChaPrngResalt(in_key, internal::kChaChaOutputBytes,
                                 &salt_counter, &position_in_buffer, &buffer));
  return absl::WrapUnique<SingleThreadChaChaPrng>(new SingleThreadChaChaPrng(
      in_key, position_in_buffer, salt_counter, std::move(buffer)));
}

rlwe::StatusOr<Uint8> SingleThreadChaChaPrng::Rand8() {
  return internal::ChaChaPrngRand8(key_, &position_in_buffer_, &salt_counter_,
                                   &buffer_);
}

rlwe::StatusOr<Uint64> SingleThreadChaChaPrng::Rand64() {
  return internal::ChaChaPrngRand64(key_, &position_in_buffer_, &salt_counter_,
                                    &buffer_);
}

}  // namespace rlwe
