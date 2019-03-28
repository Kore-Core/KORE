/*-
 * Copyright 2014 Alexander Peslyak
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "yescrypt.h"
#include "sha256_c.h"
#include "yescrypt-best_c.h"

#define YESCRYPT_N 4096
#define YESCRYPT_R 32
#define YESCRYPT_P 1
#define YESCRYPT_T 0
#define YESCRYPT_G 0
#define YESCRYPT_FLAGS (YESCRYPT_RW | YESCRYPT_WORM)

static int32_t yescryptN = 0;

static int yescrypt_wavi(const uint8_t *passwd, size_t passwdlen, const uint8_t *salt, size_t saltlen, uint8_t *buf, size_t buflen)
{
    static __thread int initialized = 0;
    static __thread yescrypt_shared_t shared;
    static __thread yescrypt_local_t local;
    int retval;

    if (!initialized) {
        /* "shared" could in fact be shared, but it's simpler to keep it private
         * along with "local".  It's dummy and tiny anyway. */
        if (yescrypt_init_shared(&shared, NULL, 0, 0, 0, 0, YESCRYPT_SHARED_DEFAULTS, NULL, 0))
            return -1;

        if (yescrypt_init_local(&local)) {
            yescrypt_free_shared(&shared);
            return -1;
        }

        initialized = 1;
    }

    retval = yescrypt_kdf(&shared, &local, passwd, passwdlen, salt, saltlen, yescryptN, YESCRYPT_R, YESCRYPT_P, YESCRYPT_T, YESCRYPT_G, YESCRYPT_FLAGS, buf, buflen);

    if (retval < 0) {
        yescrypt_free_local(&local);
        yescrypt_free_shared(&shared);
    }

    return retval;
}

void yescrypt_hash(const char *input, char *output)
{
    if (yescryptN <= 0)
        yescryptN = YESCRYPT_N;

    yescrypt_wavi((const uint8_t *) input, 89, (const uint8_t *) input, 89, (uint8_t *) output, 32);
}

void yescrypt_settestn(uint32_t n)
{
    yescryptN = n;
}
