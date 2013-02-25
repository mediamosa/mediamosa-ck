<?php
/**
 * @file
 * API documentation MediaMosa.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Allows to extend the global settings form of CK.
 *
 * @return array
 *   The form information to add to CK configuration.
 */
function hook_mediamosa_ck_configuration_collect() {
  $form['mediamosa_ck_example'] = array(
    '#type' => 'fieldset',
    '#title' => t('Example hook settings'),
    '#description' => t('Some description.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['mediamosa_ck_example']['mediamosa_ck_example_setting'] = array(
    '#title' => t('Some setting'),
    '#description' => t('Some description about this setting.'),
    '#type' => 'textfield',
    '#size' => 15,
    '#default_value' => variable_get('mediamosa_ck_example_setting', 'some default'),
  );

  return $form;
}

/**
 * Normal validation hook for input validation on settings form.
 */
function hook_mediamosa_ck_configuration_collect_validate($form, &$form_state) {
}

/**
 * Normal submit hook for settings form.
 */
function hook_mediamosa_ck_configuration_collect_submit($form, &$form_state) {
}

/**
 * @} End of "addtogroup hooks".
 */
