<?php
/**
 * @file
 * Any admin or user related code and hooks.
 */

/**
 * The main configuration form for MediaMosa Construction Kit.
 *
 * Uses the hook_mediamosa_ck_configuration_collect() to collect settings.
 *
 * @return array
 *   An array representing the form definition.
 *
 * @see media_mediamosa_configuration_form_validate()
 * @see media_mediamosa_configuration_form_submit()
 */
function mediamosa_ck_configuration_form() {
  // Build the settings form and enrich it with our settings collect hook.
  $form = system_settings_form(module_invoke_all('mediamosa_ck_configuration_collect'));

    // Create group.
  $form['mediamosa_ck_settings'] = array(
    '#type' => 'vertical_tabs',
  );

  // Collect the titles.
  foreach ($form as $name => $item) {
    if (empty($item['#type']) || $item['#type'] != 'fieldset') {
      continue;
    }

    // Make sure the fieldset is collapsed.
    $form[$name]['#collapsed'] = TRUE;

    // Get the title for sorting.
    $titles[$item['#title']] = $name;

    // Add group type to fieldset.
    $form[$name]['#group'] = 'mediamosa_ck_settings';
  }

  // Now sort on key.
  ksort($titles, SORT_STRING);

  // Now add weights.
  $weight = -count($titles);
  foreach ($titles as $name) {
    $form[$name]['#weight'] = $weight++;
  }

  return $form;
}

/**
 * Implements hook_form_validate().
 *
 * Form validation handler for main configuration form for MediaMosa
 * Construction Kit.
 *
 * @see media_mediamosa_configuration_form()
 */
function mediamosa_ck_configuration_form_validate($form, &$form_state) {

  // Call our (optional) validation hooks.
  module_invoke_all('mediamosa_ck_configuration_collect_validate', $form, $form_state);
}

/**
 * Implements hook_form_submit().
 *
 * Form submission handler for the main configuration form for MediaMosa
 * Construction Kit.
 *
 * @see media_mediamosa_configuration_form()
 */
function mediamosa_ck_configuration_form_submit($form, &$form_state) {

  // Call our (optional) submit hooks.
  module_invoke_all('mediamosa_ck_configuration_collect_submit', $form, $form_state);
}
