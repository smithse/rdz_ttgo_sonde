
#include "user.h"

#define TAG "user"
#include "logger.h"

#include <mbedtls/md.h>

// User management function (simple web authentication)
// As secure as it can be with just http:
// Login form creates a random preauth ticket (nonce)
// Client-side form calculates SHA256(username:preauth:password) as authenticator
// Upon receiving correct authenticator, TTGO generates session ID and replaces preauth ticket with session ID in internal cookie store
// (i.e. preauth ticket can be used only once)
// (i.e. preauth ticket can be used only once)
// TODO (optional) restrict session ID to specific client ID
// TODO (maybe) update session ID expiration when being used (i.e. expire only after X minutes of idle?)

#define USERLEN 8
#define RNDLEN 16
//#define COOKIE_SIZE (8+16+2)

struct SessionCookie {
  char value[USERLEN+RNDLEN+2];
  char userclass;    // -1: preauth; 0=r/o user, 1=admin [0 not used yet]
  unsigned long expiry;
};

const int MAX_SESSIONS = 4;
SessionCookie authCookies[MAX_SESSIONS];
int cookieCount = 0;

const unsigned long COOKIE_EXPIRY_DURATION = 30 * 60 * 1000; // 30 minutes in milliseconds
const unsigned long PREAUTH_EXPIRY_DURATION = 60 * 1000;  // 1 minute in milliseconds


void cleanupExpiredCookies() {
  unsigned long now = millis();
  int i = 0;

  // Loop through the cookies and remove expired ones
  while (i < cookieCount) {
    if (authCookies[i].expiry < now) {
      // Shift the remaining cookies left
      for (int j = i; j < cookieCount - 1; j++) {
        authCookies[j] = authCookies[j + 1];
      }
      cookieCount--; // Reduce the count of cookies
    } else {
      i++;
    }
  }
}

void storeCookie(const char *cookie, char userclass) {
  cleanupExpiredCookies();
  if (cookieCount >= MAX_SESSIONS) {
    LOG_D(TAG, "No space for new cookie. Cleaning up oldest cookies.");
    for(int i=1; i<MAX_SESSIONS; i++) { 
      authCookies[i-1] = authCookies[i];
    }
    cookieCount--;
  }
  // Copy cookie string into the fixed-size array
  strlcpy(authCookies[cookieCount].value, cookie, sizeof(authCookies[cookieCount].value));
  authCookies[cookieCount].userclass = userclass;
  authCookies[cookieCount].expiry = millis() + (userclass==-1?PREAUTH_EXPIRY_DURATION:COOKIE_EXPIRY_DURATION);
  cookieCount++;
  LOG_D(TAG, "Cookie stored: %s, Expiry in: %lu ms\n", cookie, COOKIE_EXPIRY_DURATION);
}

int upgradeCookie(const char *preauth, const char *cookie, char userclass) {
  for (int i = 0; i < cookieCount; i++) {
    if (strcmp(preauth, authCookies[i].value)==0) {
      strlcpy(authCookies[i].value, cookie, sizeof(authCookies[i].value));
      return 0;
    }
  }
  return -1;
}

char allcn[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Function to generate a random session cookie
void generateRandomCookie(const char *user, char *cookie) {
  int j = 0;
  for (int i = 0; i < USERLEN && user[i]; i++, j++) {
    cookie[j] = user[i];
  }
  cookie[j++] = ':';
  for (int i = 0; i < RNDLEN; ++i) {
    cookie[j++] = allcn[random(sizeof(allcn)-1)];  // sizeof includes \0
  }
  cookie[j++] = '\0';
}



bool isCookieValid(const char *cookie) {
  unsigned long now = millis();
  for (int i = 0; i < cookieCount; i++) {
    if (strcmp(authCookies[i].value, cookie) == 0) {
      if (authCookies[i].expiry > now) {
        return true; // Valid and not expired
      } else {
        // Cookie expired, remove it
        for (int j = i; j < cookieCount - 1; j++) {
          authCookies[j] = authCookies[j + 1];
        }
        cookieCount--; // Decrement cookie count
        break; // Exit the loop after removing expired cookie
      }
    }
  }
  return false; // Not found or expired
}

bool isValidUser(const char *user, const char *preauth, const char *auth) {
    // simple digest authentication:
    // (we want to avoid sending plain text passwords via http)
    // digest is SHA256(user:preauth:password)
    char buf[256];
    strlcpy(buf, user, 256);
    strlcat(buf, ":", 256);
    strlcat(buf, preauth, 256);  // TODO: Check if it exists? (well upgrade will fail if not...)
    strlcat(buf, ":", 256);   // TODO bound checks.... this is user provided data!!!!!!
    strlcat(buf, "1234", 256);  // TODO: pick password for user
    unsigned char sharesult[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *) buf, strlen(buf));
    mbedtls_md_finish(&ctx, sharesult);
    bool match = true;
    Serial.print("Hash: ");
    Serial.printf(" Comparing to %s\n", auth);
    for(int i= 0; i< sizeof(sharesult); i++){
      char str[3];
      sprintf(str, "%02x", (int)sharesult[i]);
      Serial.printf(" [%c-%c-%c-%c] ",auth[0], auth[1], str[0], str[1]);
      if( (auth[0]!=str[0]) || (auth[1]!=str[1])) match = false;
      auth += 2;
      Serial.print(str);
    } 
    Serial.printf("Match: %d\n", match);
    return match;
}

