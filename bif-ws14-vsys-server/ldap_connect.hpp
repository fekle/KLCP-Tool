
#include <ldap.h>

#define LDAP_HOST "ldap.technikum-wien.at"
#define LDAP_PORT 389
#define SEARCHBASE "dc=technikum-wien,dc=at"
#define SCOPE LDAP_SCOPE_SUBTREE
#define BIND_USER NULL        /* anonymous bind with user and pw NULL */
#define BIND_PW NULL

class ldap_auth {
private:
    int x;
public:
    void print_hello();
    bool establish_ldap_auth(std::string, std::string);
};

void ldap_auth::print_hello() {
    std::cout << "Connecting with " << LDAP_HOST << " on port " << LDAP_PORT << std::endl;
}

bool establish_ldap_auth(std::string username, std::string password) {

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
        std::cout << "Error initialising LDAP connection" << std::endl;
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
            std::cout << "LDAP Search Error" << std::endl;
            return EXIT_FAILURE;
        }

        ld_count_entries = ldap_count_entries(ld, result);

        if (ld_count_entries == 1) {
            e = ldap_first_entry(ld, result);
            strcat(dn, ldap_get_dn(ld, e));
            std::cout << " DN from search: " << dn << std::endl;
        }
        else if (ld_count_entries == 0) {

            std::stringstream dn;
            dn << "uid=" << username << ",ou=People,dc=technikum-wien,dc=at";
            std::string dn_string = dn.str();
            std::cout << "Standard DN: " << dn_string << std::endl;
        }
        else {
            std::cout << "Invalid number of results: " << ld_count_entries << std::endl;
            return EXIT_FAILURE;
        }
    }
    else {
        return EXIT_FAILURE;
    }

    int check_login = ldap_simple_bind_s(ld, dn, passed_password);

    if (check_login == LDAP_SUCCESS) {
        return true;
    }
    else {
        return false;
    }

    ldap_msgfree(result);
    free(attribs[0]);
    free(attribs[1]);
    ldap_unbind(ld);
}