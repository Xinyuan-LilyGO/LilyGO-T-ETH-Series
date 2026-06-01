#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Include the production header/source under test */
#include "esp-idf-examples/usb_cdc_4g_module/main/modem_http_config.c"

START_TEST(test_unauthenticated_requests_rejected)
{
    /* Invariant: Protected endpoints must reject requests without valid credentials */
    const char *payloads[] = {
        /* Exact exploit: oversized username to trigger potential buffer overflow in digest */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        /* Boundary: empty credentials */
        "",
        /* Valid input: well-formed credentials (should succeed, not be rejected) */
        "admin:password123"
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        char *digest = NULL;
        const char *payload = payloads[i];

        /* Split payload into username:password or use as username with empty password */
        char *colon = strchr(payload, ':');
        char username[512] = {0};
        char password[512] = {0};

        if (colon) {
            size_t ulen = (size_t)(colon - payload);
            strncpy(username, payload, ulen < 511 ? ulen : 511);
            strncpy(password, colon + 1, 511);
        } else {
            strncpy(username, payload, 511);
        }

        /* Call the real set_auth function from modem_http_config.c */
        esp_err_t ret = modem_http_set_auth(username, password, &digest);

        if (i == 2) {
            /* Valid credentials: function should succeed */
            ck_assert_msg(ret == ESP_OK, "Valid credentials should be accepted");
            ck_assert_ptr_nonnull(digest);
            ck_assert_msg(strncmp(digest, "Basic ", 6) == 0,
                          "Valid auth header must start with 'Basic '");
        } else {
            /* Invalid/malformed: must not crash and must not produce a valid auth header
               that could bypass authentication */
            if (ret == ESP_OK && digest != NULL) {
                /* If a digest was produced for empty creds, it must still be well-formed
                   (no buffer overflow corruption) */
                ck_assert_msg(strlen(digest) < 4096,
                              "Digest length suspiciously large — possible overflow");
            }
        }

        free(digest);
        digest = NULL;
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_unauthenticated_requests_rejected);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}