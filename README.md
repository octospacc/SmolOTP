# SmolOTP

Barebones OTP application designed for immediate usage on the simplest on devices.

Download binary releases: <https://gitlab.com/octospacc/SmolOTP/-/releases>

## Current Support

Platforms:
* Nintendo DS

OTP protocols:
* TOTP

## Planned Features

* More platforms (via LibMultiSpacc)
* Encryption of secrets, unlock via fast-usable key combos
* Changing options (time configuration, adding secrets, ...) in-app
* Better gathering of time/date with less configuration needed by users
* Handling unlimited secrets (currently there is a 256 hard limit + much lower soft limit because of no screen scrolling lol)

## Credits

This app integrates the following third-party libraries:

* TOTP protocol implementation: <https://github.com/fmount/c_otp>
* HMAC+SHA1 protocol implementation: <https://github.com/kokke/tiny-HMAC-c>
* INI configuration parser: <https://github.com/benhoyt/inih>
