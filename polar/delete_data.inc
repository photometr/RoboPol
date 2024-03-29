<?php

/**
 * 'Delete entries' form
 * Allows to delete list of dates from the DB
 */
function polar_form_delete($form, &$form_state) {
  //check if user allowed to add to the database
  global $user;
  if (! in_array("robopol", $user->roles)) {
    $error_text = '<font size="3" color="red">Only registered users are able to access the data!</font>';
    $form['access_fieldset'] = array(
      '#type' => 'fieldset',
      '#title' => t($error_text),
      '#description' => t("Please contact RoboPol team to get the access."),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    return $form;
  }
  $form = array();

  $form = array(
    '#prefix' => '<div id="updateform">',
    '#suffix' => '</div>',
  );
  $form['name'] = array(
    '#type' => 'select',
    '#options' => blazars_list_load(),
    '#title' => t('Blazar name:'),
  );
  $form['data'] = array(
    '#type'  => 'textarea',
    '#title' => 'Dates for removal:',
    '#size'  => 15,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}


/**
 * Submit handler for 'delete entries' form.
 */
function polar_form_delete_submit($form, &$form_state) {
  $entry = array(
    'blazar_name' => $form_state['values']['name'],
    'data'        => $form_state['values']['data']
  );
  $return = polar_entries_delete($entry);
  if ($return) {
    drupal_set_message(t("Successfully added"));
  }
}

/**
 * Handler for 'delete entries' requests
 */
function polar_entries_delete($entry) {
  global $user;
  //check if user allowed to add the database
  if (! in_array("robopol", $user->roles)) {
    drupal_set_message(t('ask Admin to be added to robopol role'), 'error');
    return NULL;
  }
  $query = polar_entries_delete_polar($entry);

  return $query;
}

/**
 * Delete polarimetry entries from the database.
 *
 */
function polar_entries_delete_polar($entry) {
  global $user;

  $blazar_id = $entry['blazar_name'];

  //parsing data
  $data_arr = preg_split("/((?<!\\\|\r)\n)|((?<!\\\)\r\n)/", $entry['data']);
  foreach ($data_arr as $value) {
    $value = trim ($value);
    if ($value == "") continue;
    $one_line  = preg_split("/[\s,]+/", $value);

    if (!(is_numeric($one_line[0]))) {
      drupal_set_message(t('It seems that you give not numeric data.'), 'error');
      return NULL;
    }
    $JD[] = floatval($one_line[0]);
  }

  $trans_value = db_transaction();
  try {
    foreach ($JD as $date) {
      $result = db_delete('polar_polar')
                   ->condition('blazar_id', $blazar_id)
                   ->condition('jd', $date)
                   ->execute();
    }
    $result = True;
  }
  catch (Exception $e) {
    $trans_value->rollback();
    drupal_set_message(t('db_delete failed. Message = %message, query= %query',
      array('%message' => $e->getMessage(), '%query' => $e->query_string)), 'error');
    $result = False;
  }
  return $result;
}
