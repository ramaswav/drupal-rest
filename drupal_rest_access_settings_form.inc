<?php

require_once ( DRUPAL_ROOT. '/vendor/firebase/php-jwt/src/JWT.php');

/**
 * Render a settings form.
 * Also display existing  data.
 */
function drupal_rest_config($form, &$form_state) {

  $form['#tree'] = TRUE;

  if (empty($form_state['number_names'])) {
    $form_state['number_names'] = 1;
  }

  for ($i = 1; $i <= $form_state['number_names']; $i++) {

    $form['container'][$i] = array(
      '#type' => 'container',
    );
    $form['container'][$i]['consumer_name'] = array(
      '#type' => 'textfield',
      '#title' => 'Enter Resource Consumer Name',
      '#attributes' =>array('placeholder' => t('Resource Consumer Name')),
      '#size' => 20,
      '#required' => TRUE,
    );
  }

  $query = db_query("SELECT v.name, v.value FROM  variable v where v.name like 'consumer%'");
  $result = $query->fetchAll();
  $display_text='';
  foreach ($result as $record) {
    $display_text .= $record->name . "=" . $record->value. PHP_EOL;
  }

  $form['container'][$form_state['number_names']]['existing_list'] = array(
    '#type' => 'textarea',
    '#value' => $display_text,
    '#size' => 500,
    '#title' => 'Existing Keys, Re-enter key/value to override',
    '#required' => FALSE,
    '#disabled' => FALSE,
  );

  $form['add_item'] = array(
    '#type' => 'submit',
    '#value' => t('Add another set'),
    '#submit' => array('drupal_rest_access_add_item'),
  );


  if ($form_state['number_names'] > 1) {

    $form['remove_item'] = array(
      '#type' => 'submit',
      '#value' => t('Remove latest name'),
      '#submit' => array('drupal_rest_access_remove_item'),
      // Since we are removing a name, don't validate until later.
      '#limit_validation_errors' => array(),
    );
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' =>  'Generate JSON Web Token',
    '#submit' =>  array('drupal_rest_access_form_submit'),
  );

  return $form;
}

/**
 * Add new customer fields().
 */
function drupal_rest_access_add_item($form, &$form_state) {

  // All we need is to increase the number used in our for loop above ...
  $form_state['number_names']++;
  // ... and rebuild our form.
  $form_state['rebuild'] = TRUE;
}

/**
 * Remove previous fields().
 */
function drupal_rest_access_remove_item($form, &$form_state) {

  // Just the other way around until only one item is left ...
  if ($form_state['number_names'] > 1) {
    $form_state['number_names']--;
  }
  // ... and form rebuild again.
  $form_state['rebuild'] = TRUE;
}

/**
 * Implements hook_form_submit().
 * Saves the valid customer secret keys in variables table
 * For now.
 */
function drupal_rest_access_form_submit($form, $form_state) {
  foreach ($form_state['values']['container'] as $consumer) {
    variable_set('consumer-'.$consumer['consumer_name'],drupal_rest_jwt_token($consumer));
  }
  drupal_set_message("successfully saved access key settings");
}


function drupal_rest_jwt_token($consumer_name) {
  $tokenId    = base64_encode(mcrypt_create_iv(32));
  $issuedAt   = time();
  $notBefore  = $issuedAt + 10;             //Adding 10 seconds
  $expire     = $notBefore + 60;            // Adding 60 seconds
  $serverName = 'localhost'; // Retrieve the server name from config file

  /*
   * Create the token as an array
   */
  $data = [
    'iat'  => $issuedAt,         // Issued at: time when the token was generated
    'jti'  => $tokenId,          // Json Token Id: an unique identifier for the token
    'iss'  => $serverName,       // Issuer
    'nbf'  => $notBefore,        // Not before
    'exp'  => $expire,           // Expire
    'data' => [                  // Data related to the signer user
      'userId'   => $consumer_name, // userid from the users table
      'userName' => $consumer_name // User name
    ]
  ];

  $jwt = \Firebase\JWT\JWT::encode(
    $data,      //Data to be encoded in the JWT
    'drupalrest', // The signing key
    'HS512'     // Algorithm used to sign the token, see https://tools.ietf.org/html/draft-ietf-jose-json-web-algorithms-40#section-3
  );

  return $jwt;
}
