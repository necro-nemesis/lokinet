#pragma once

#include "constants.hpp"
#include <llarp/router_id.hpp>
#include <llarp/util/aligned.hpp>
#include <llarp/util/types.hpp>
#include <llarp/util/fs.hpp>

#include <algorithm>
#include <iostream>

namespace llarp
{
  using SharedSecret = AlignedBuffer<SHAREDKEYSIZE>;
  using KeyExchangeNonce = AlignedBuffer<32>;

  struct PubKey final : public AlignedBuffer<PUBKEYSIZE>
  {
    PubKey() = default;

    explicit PubKey(const byte_t* ptr) : AlignedBuffer<SIZE>(ptr)
    {}

    explicit PubKey(const Data& data) : AlignedBuffer<SIZE>(data)
    {}

    explicit PubKey(const AlignedBuffer<SIZE>& other) : AlignedBuffer<SIZE>(other)
    {}

    std::string
    ToString() const;

    bool
    FromString(const std::string& str);

    operator RouterID() const
    {
      return {as_array()};
    }

    PubKey&
    operator=(const byte_t* ptr)
    {
      std::copy(ptr, ptr + SIZE, begin());
      return *this;
    }
  };

  inline bool
  operator==(const PubKey& lhs, const PubKey& rhs)
  {
    return lhs.as_array() == rhs.as_array();
  }

  inline bool
  operator==(const PubKey& lhs, const RouterID& rhs)
  {
    return lhs.as_array() == rhs.as_array();
  }

  inline bool
  operator==(const RouterID& lhs, const PubKey& rhs)
  {
    return lhs.as_array() == rhs.as_array();
  }

  struct PrivateKey;

  /// Stores a sodium "secret key" value, which is actually the seed
  /// concatenated with the public key.  Note that the seed is *not* the private
  /// key value itself, but rather the seed from which it can be calculated.
  struct SecretKey final : public AlignedBuffer<SECKEYSIZE>
  {
    SecretKey() = default;

    explicit SecretKey(const byte_t* ptr) : AlignedBuffer<SECKEYSIZE>(ptr)
    {}

    // The full data
    explicit SecretKey(const AlignedBuffer<SECKEYSIZE>& seed) : AlignedBuffer<SECKEYSIZE>(seed)
    {}

    // Just the seed, we recalculate the pubkey
    explicit SecretKey(const AlignedBuffer<32>& seed)
    {
      std::copy(seed.begin(), seed.end(), begin());
      Recalculate();
    }

    /// recalculate public component
    bool
    Recalculate();

    std::string_view
    ToString() const
    {
      return "[secretkey]";
    }

    PubKey
    toPublic() const
    {
      return PubKey(data() + 32);
    }

    /// Computes the private key from the secret key (which is actually the
    /// seed)
    bool
    toPrivate(PrivateKey& key) const;

    bool
    LoadFromFile(const fs::path& fname);

    bool
    SaveToFile(const fs::path& fname) const;
  };

  /// PrivateKey is similar to SecretKey except that it only stores the private
  /// key value and a hash, unlike SecretKey which stores the seed from which
  /// the private key and hash value are generated.  This is primarily intended
  /// for use with derived keys, where we can derive the private key but not the
  /// seed.
  struct PrivateKey final : public AlignedBuffer<64>
  {
    PrivateKey() = default;

    explicit PrivateKey(const byte_t* ptr) : AlignedBuffer<64>(ptr)
    {}

    explicit PrivateKey(const AlignedBuffer<64>& key_and_hash) : AlignedBuffer<64>(key_and_hash)
    {}

    /// Returns a pointer to the beginning of the 32-byte hash which is used for
    /// pseudorandomness when signing with this private key.
    const byte_t*
    signingHash() const
    {
      return data() + 32;
    }

    /// Returns a pointer to the beginning of the 32-byte hash which is used for
    /// pseudorandomness when signing with this private key.
    byte_t*
    signingHash()
    {
      return data() + 32;
    }

    std::string_view
    ToString() const
    {
      return "[privatekey]";
    }

    /// Computes the public key
    bool
    toPublic(PubKey& pubkey) const;
  };

  /// IdentitySecret is a secret key from a service node secret seed
  struct IdentitySecret final : public AlignedBuffer<32>
  {
    IdentitySecret() : AlignedBuffer<32>()
    {}

    /// no copy constructor
    explicit IdentitySecret(const IdentitySecret&) = delete;
    // no byte data constructor
    explicit IdentitySecret(const byte_t*) = delete;

    /// load service node seed from file
    bool
    LoadFromFile(const fs::path& fname);

    std::string_view
    ToString() const
    {
      return "[IdentitySecret]";
    }
  };

  template <>
  constexpr inline bool IsToStringFormattable<PubKey> = true;
  template <>
  constexpr inline bool IsToStringFormattable<SecretKey> = true;
  template <>
  constexpr inline bool IsToStringFormattable<PrivateKey> = true;
  template <>
  constexpr inline bool IsToStringFormattable<IdentitySecret> = true;

  using ShortHash = AlignedBuffer<SHORTHASHSIZE>;
  using LongHash = AlignedBuffer<HASHSIZE>;

  struct Signature final : public AlignedBuffer<SIGSIZE>
  {
    byte_t*
    Hi();

    const byte_t*
    Hi() const;

    byte_t*
    Lo();

    const byte_t*
    Lo() const;
  };

  using TunnelNonce = AlignedBuffer<TUNNONCESIZE>;
  using SymmNonce = AlignedBuffer<NONCESIZE>;
  using SymmKey = AlignedBuffer<32>;

  using PQCipherBlock = AlignedBuffer<PQ_CIPHERTEXTSIZE + 1>;
  using PQPubKey = AlignedBuffer<PQ_PUBKEYSIZE>;
  using PQKeyPair = AlignedBuffer<PQ_KEYPAIRSIZE>;

  /// PKE(result, publickey, secretkey, nonce)
  using path_dh_func =
      std::function<bool(SharedSecret&, const PubKey&, const SecretKey&, const TunnelNonce&)>;

  /// TKE(result, publickey, secretkey, nonce)
  using transport_dh_func =
      std::function<bool(SharedSecret&, const PubKey&, const SecretKey&, const TunnelNonce&)>;

  /// SH(result, body)
  using shorthash_func = std::function<bool(ShortHash&, const llarp_buffer_t&)>;
}  // namespace llarp

namespace std
{
  template <>
  struct hash<llarp::PubKey> : hash<llarp::AlignedBuffer<llarp::PubKey::SIZE>>
  {};
};  // namespace std
