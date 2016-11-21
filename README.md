# drupal-rest
Drupal Rest/JWT: This module exposes drupal resources as json, the resource consumers is configurable. Uses JWT token based authentication.

#Installation
Do a composer install inside the module directory, this is to install https://github.com/firebase/php-jwt, for token authentication.

```
cd sites/all/modules/custom/drupal_rest
composer install
drush en -y drupal-rest
```
