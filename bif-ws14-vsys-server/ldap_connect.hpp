#include <ldap.h>
#include "BanIP.hpp"

#define LDAP_HOST "ldap.technikum-wien.at"
#define LDAP_PORT 389
#define SEARCHBASE "dc=technikum-wien,dc=at"
#define SCOPE LDAP_SCOPE_SUBTREE
#define BIND_USER NULL        /* anonymous bind with user and pw NULL */
#define BIND_PW NULL
#define BANNED_FOR_TIME 60

class ldap_auth {
private:
    int login_attempts = 0;
public:
    bool establish_ldap_auth(std::string, std::string, std::string, banned_ip *);
};

bool ldap_auth::establish_ldap_auth(std::string username, std::string password, std::string _ip_address, banned_ip *ban) {

    LDAP *ld;
    LDAPMessage *result, *e;
    char uid[] = "uid";
    char cn[] = "cn";
    char *attribs[3];        /* attribute array for search */

    attribs[0] = uid;            /* return uid and cn of entries */
    attribs[1] = cn;
    attribs[2] = NULL;        /* array must be NULL terminated */

    int rc = 0;
    int ld_count_entries = 0;
    const char *passed_password = password.c_str();
    /* Setup LDAP Connection */
    if ((ld = ldap_init(LDAP_HOST, LDAP_PORT)) == NULL) {
        printError("Error initialising LDAP connection");
        return EXIT_FAILURE;
    }

    char dn[47] = "";

    /* Anonymous bind */
    rc = ldap_simple_bind_s(ld, BIND_USER, BIND_PW);

    if (rc == LDAP_SUCCESS) {

        std::stringstream ss_filter;
        ss_filter << "(uid=" << username << ")";

        std::string s_filter = ss_filter.str();

        rc = ldap_search_s(ld, SEARCHBASE, 0, s_filter.c_str(), attribs, 0, &result);

        if (rc != LDAP_SUCCESS) {
            printError("LDAP Search Error");
            return EXIT_FAILURE;
        }

        ld_count_entries = ldap_count_entries(ld, result);

        if (ld_count_entries == 1) {
            e = ldap_first_entry(ld, result);
            strcat(dn, ldap_get_dn(ld, e));
            std::stringstream ss;
            ss << " DN from search: " << dn << std::endl;
            printInfo(ss.str());
        } else if (ld_count_entries == 0) {

        } else {
            std::stringstream ss;
            ss << "Invalid number of results: " << ld_count_entries << std::endl;
            printError(ss.str());
            return EXIT_FAILURE;
        }
    } else {
        printError("LDAP connection Failed");
        return EXIT_FAILURE;
    }

    int check_login = ldap_simple_bind_s(ld, dn, passed_password);

    bool isbanned = ban->the_ban(_ip_address);


    if (!isbanned) {
        if (check_login == LDAP_SUCCESS) {
            return true;
        } else {
            login_attempts++;
            if (login_attempts == 3) {
                time_t bantime = BANNED_FOR_TIME;
                ban->is_banned(_ip_address, bantime);
            }
            return false;
        }
    } else {
        return false;
    }

    ldap_msgfree(result);
    free(attribs[0]);
    free(attribs[1]);
    ldap_unbind(ld);
}