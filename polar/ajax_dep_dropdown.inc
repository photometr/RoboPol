<?php

/**
 * @file
 * AJAX
 */

/**
 * @defgroup polar
 * @ingroup RoboPol
 * @{
 * These examples show AJAX with graceful degradation when Javascript is not
 * available.
 *
 * In each of these the key idea is that the form is rebuilt different ways
 * depending on form input. In order to accomplish that, the formbuilder function
 * is in charge of almost all logic.
 */

/**
 * A form with a dropdown whose options are dependent on a
 * choice made in a previous dropdown.
 *
 * On changing the first dropdown, the options in the second
 * are updated. Gracefully degrades if no javascript.
 *
 * A bit of CSS and javascript is required. The CSS hides the "add more" button
 * if javascript is not enabled. The Javascript snippet is really only used
 * to enable us to present the form in degraded mode without forcing the user
 * to turn off Javascript.  Both of these are loaded by using the
 * #attached FAPI property, so it is a good example of how to use that.
 *
 * The extra argument $no_js_use is here only to allow presentation of this
 * form as if Javascript were not enabled. ajax_example_menu() provides two
 * ways to call this form, one normal ($no_js_use = FALSE) and one simulating
 * Javascript disabled ($no_js_use = TRUE).
 */
function polar_form_get($form, &$form_state) {
  //check if user allowed to add the database
  global $user;
  if (! in_array("robopol", $user->roles)) {
    $error_text = '<font size="3" color="red">Only registered users are able to access the data!</font>';
    $form['access_fieldset'] = array(
      '#type' => 'fieldset',
      '#title' => t($error_text),
      '#description' => t("Please <a href='http://robopol.org/user/login'>login in</a> or contact the RoboPol team to get the access."),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      
    );
    return $form;
  }
  drupal_add_js('misc/dygraphs/dygraph-dev.js');
  drupal_add_css(drupal_get_path('module', 'Fermi') . '/JDreplwidth.css', array('group' => CSS_DEFAULT, 'type' => 'file'));

  // Get the list of options to populate the first dropdown.
  $blazars_names = blazars_list_load();

  // If we have a value for the first dropdown from $form_state['values'] we use
  // this both as the default value for the first dropdown and also as a
  // parameter to pass to the function that retrieves the options for the
  // second dropdown.
  $selected = isset($form_state['values']['name']) ? $form_state['values']['name'] : key($blazars_names);

  $form['description'] = array(
    '#markup' => '<div>' . t("Here you may download RoboPol's polarimetric data. These data are fake now (generated for test purposes only).")
    . '</div>',
  );

  $form['name_fieldset'] = array(
    '#type' => 'fieldset',
  );
  $form['name_fieldset']['name'] = array(
    '#type' => 'select',
    '#title' => 'Blazar name:',
    '#options' => $blazars_names,
    '#attributes' => array('class' => array('enabled-for-ajax')),

    // The '#ajax' property allows us to bind a callback to the server whenever this
    // form element changes. See ajax_example_autocheckboxes and
    // ajax_example_dependent_dropdown in ajax_example.module for more details.
    '#ajax' => array(
      'callback' => 'ajax_name_dropdown_callback',
      'wrapper' => 'JD-replace',//dropdown-second-replace
    ),
  );

  $form['JD_fieldset'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div id="JD-replace">',
    '#suffix' => '</div>',
  );
  $form['JD_fieldset']['JDstart'] = array(
    '#type'  => 'textfield',
    '#title' => t('From JD:'),
    '#size'  => 10,
    '#attributes' => array('class' => array('enabled-for-ajax')),
    '#default_value' => '',
    '#prefix' => '<div id="JD-start" class="datesform">',
    '#suffix' => '</div>',
  );
  $form['JD_fieldset']['JDend'] = array(
    '#type'  => 'textfield',
    '#title' => t('To JD:'),
    '#size'  => 10,
    '#description' => t('leave fields blank for the whole dataset'),
    '#prefix' => '<div id="JD-end" class="datesform">',
    '#suffix' => '</div>',
  );
  $form['JD_fieldset']['plot'] = array(
    '#prefix' => '<div id="plot_div">',
    '#suffix' => '</div>',
    //'#markup' => get_plot_code_polar($form, $form_state),
    //'#markup' => '<div id="chart3" style="margin-top:20px; margin-left:20px; width:500px; height:300px;"></div>',
    '#attributes' => array('class' => array('enabled-for-ajax')),
  );
  $form['JD_fieldset']['ASCII'] = array(
    '#type' => 'submit',
    '#value' => t('Load data'),
    '#submit' => array('polar_form_get_ASCII_submit'),
    // This class allows attached js file to override the disabled attribute,
    // since it's not necessary in ajax-enabled form.
    '#attributes' => array('class' => array('enabled-for-ajax')),
  );
  //$form['JD_fieldset']['VOT'] = array(
  //  '#type' => 'submit',
  //  '#value' => t('Скачать VOT'),
  //  '#submit' => array('polar_form_get_VOT_submit'),
  //  '#attributes' => array('class' => array('enabled-for-ajax')),
  //);

  // Disable band if a selection has not been made on dropdown_first.
  if (empty($form_state['values']['name'])) {
    $form['JD_fieldset']['JDstart']['#disabled'] = TRUE;
    $form['JD_fieldset']['JDend']['#disabled'] = TRUE;
    $form['JD_fieldset']['ASCII']['#disabled'] = TRUE;
    $form['JD_fieldset']['VOT']['#disabled'] = TRUE;
    $form['JD_fieldset']['bibtex']['#disabled'] = TRUE;
  }

  return $form;
}


/**
 * Selects just the second dropdown to be returned for re-rendering.
 *
 * @return
 *   Renderable array (the second dropdown).
 */
function ajax_name_dropdown_callback($form, $form_state) {
  $form['JD_fieldset']['plot']['#markup'] = get_plot_code_polar($form, $form_state);
  return $form['JD_fieldset'];
}

/**
 * @} End of "defgroup polar".
 */
