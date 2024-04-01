#pragma once

#include "dns.h"

#include "stdint.h"
#include "optional"

/**
 * Translates name (like 'google.com{0x0}') to query ({0x6}google{0x3}com{0x0})
 * right in place.
 * 
 * NB: this function needs to be called on byte before the name.
 */
void
name_to_query(char *name);

/**
 * Reads from @a fd to @a buf of @a buf_size size.
 * The function tries to read until @a offsete will reach @a required_offset.
 * If underlying read fails, the function fails as well.
 * 
 * Argument @a offset is a state of the function - when using it, one should
 * initialize an offset variable to 0 and never touch it manually.
 * 
 * Returns 0 on success, -1 on error.
 */
int
readn(int fd, char *buf, size_t *offset, size_t required_offset, size_t buf_size);

/**
 * Reads and "validates" incoming dns request.
 * 
 * NB: the function expects the connection to be established over TCP,
 * because it reads two extra bytes (packet length) accorging to DNS
 * specification.
 * 
 * Returns non-empty result on success, std::nullopt on error.
 */
std::optional<dns_packet>
read_request(int fd);

/**
 * Writes DNS response over TCP (with 2 extra bytes at the beginning - they
 * are considered as packet length).
 */
bool
write_answer(int fd, dns_packet &p);
