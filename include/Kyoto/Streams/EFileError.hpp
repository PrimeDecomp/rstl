#ifndef KYOTO_STREAMS_EFILEERROR_HPP
#define KYOTO_STREAMS_EFILEERROR_HPP

// Guessed name. Results of the host file-I/O calls.

enum EFileError {
  kFE_OK,
  kFE_CantInitialize,  // Guessed name
  kFE_TimedOut,        // Guessed name
  kFE_InvalidOpenMode, // Guessed name
  kFE_CantOpen,        // Guessed name
  kFE_CantClose,       // Guessed name
  kFE_CouldntRead,     // Guessed name
  kFE_PartialRead,     // Guessed name
  kFE_CouldntWrite,    // Guessed name
  kFE_PartialWrite,    // Guessed name
  kFE_CouldntSeek,     // Guessed name
};

#endif
