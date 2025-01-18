
#define COOKIE_SIZE 26

void cleanupExpiredCookies();
void storeCookie(const char *cookie, char userclass);
int upgradeCookie(const char *preauth, const char *cookie, char userclass);
void generateRandomCookie(const char *user, char *cookie);
bool isCookieValid(const char *cookie);
bool isValidUser(const char *user, const char *preauth, const char *auth);
