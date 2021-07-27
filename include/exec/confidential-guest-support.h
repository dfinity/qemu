/*
 * QEMU Confidential Guest support
 *   This interface describes the common pieces between various
 *   schemes for protecting guest memory or other state against a
 *   compromised hypervisor.  This includes memory encryption (AMD's
 *   SEV and Intel's MKTME) or special protection modes (PEF on POWER,
 *   or PV on s390x).
 *
 * Copyright Red Hat.
 *
 * Authors:
 *  David Gibson <david@gibson.dropbear.id.au>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or
 * later.  See the COPYING file in the top-level directory.
 *
 */
#ifndef QEMU_CONFIDENTIAL_GUEST_SUPPORT_H
#define QEMU_CONFIDENTIAL_GUEST_SUPPORT_H

#ifndef CONFIG_USER_ONLY

#include "qom/object.h"

#define TYPE_CONFIDENTIAL_GUEST_SUPPORT "confidential-guest-support"
OBJECT_DECLARE_SIMPLE_TYPE(ConfidentialGuestSupport, CONFIDENTIAL_GUEST_SUPPORT)

struct ConfidentialGuestSupport {
    Object parent;

    /*
     * ready: flag set by CGS initialization code once it's ready to
     *        start executing instructions in a potentially-secure
     *        guest
     *
     * The definition here is a bit fuzzy, because this is essentially
     * part of a self-sanity-check, rather than a strict mechanism.
     *
     * It's not feasible to have a single point in the common machine
     * init path to configure confidential guest support, because
     * different mechanisms have different interdependencies requiring
     * initialization in different places, often in arch or machine
     * type specific code.  It's also usually not possible to check
     * for invalid configurations until that initialization code.
     * That means it would be very easy to have a bug allowing CGS
     * init to be bypassed entirely in certain configurations.
     *
     * Silently ignoring a requested security feature would be bad, so
     * to avoid that we check late in init that this 'ready' flag is
     * set if CGS was requested.  If the CGS init hasn't happened, and
     * so 'ready' is not set, we'll abort.
     */
    bool ready;
};

/**
 * The functions registers with ConfidentialGuestMemoryEncryptionOps will be
 * used during the encrypted guest migration.
 */
struct ConfidentialGuestMemoryEncryptionOps {
    /* Initialize the platform specific state before starting the migration */
    int (*save_setup)(const char *pdh, const char *plat_cert,
                      const char *amd_cert);

    /* Write the encrypted page and metadata associated with it */
    int (*save_outgoing_page)(QEMUFile *f, uint8_t *ptr, uint32_t size,
                              uint64_t *bytes_sent);

    /* Load the incoming encrypted page into guest memory */
    int (*load_incoming_page)(QEMUFile *f, uint8_t *ptr);

    /* Check if gfn is in shared/unencrypted region */
    bool (*is_gfn_in_unshared_region)(unsigned long gfn);

    /* Write the shared regions list */
    int (*save_outgoing_shared_regions_list)(QEMUFile *f);

    /* Load the shared regions list */
    int (*load_incoming_shared_regions_list)(QEMUFile *f);
};

typedef struct ConfidentialGuestSupportClass {
    ObjectClass parent;
    struct ConfidentialGuestMemoryEncryptionOps *memory_encryption_ops;
} ConfidentialGuestSupportClass;

#endif /* !CONFIG_USER_ONLY */

#endif /* QEMU_CONFIDENTIAL_GUEST_SUPPORT_H */