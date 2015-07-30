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
 */
function mediamosa_ck_configuration_form() {
  // Build the settings form and enrich it with our settings collect hook.
  $form = system_settings_form(module_invoke_all('mediamosa_ck_configuration_collect'));

  // Create group.
  $form['mediamosa_ck_settings'] = array(
    '#type' => 'vertical_tabs',
  );

  // Collect the titles.
  $titles = array();
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

  $form['mediamosa_ck_settings']['owner'] = array(
    '#type' => 'fieldset',
    '#title' => 'Ownership',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Setting to identify the owner in the MediaMosa'),
    '#weight' => 10,
  );

  $form['mediamosa_ck_settings']['owner']['mediamosa_ck_asset_owner'] = array(
    '#type' => 'radios',
    '#title' => t('owner field'),
    '#default_value' => variable_get('mediamosa_ck_asset_owner', 'mail'),
    '#options' => array('name' => t('Drupal username'), 'mail' => t('Drupal user mail')),
    '#description' => t('Either the user name or the user email address is used to store as owner of the asset.'),
  );

  // Add Transcodes settings here. This resides here instead of in the
  // mediamosa_ck_transcode module, as this is an very important setting, and
  // almost always used. The transcode module containes further transcoding
  // functions, for advanced use.
  $form['mediamosa_ck_settings']['transcode_profile'] = array(
    '#type' => 'fieldset',
    '#title' => 'Transcode profiles',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Setting page on how to use and restrict MediaMosa transcode profiles.'),
    '#weight' => 11,
  );

  // Build up the options.
  $options = array(
    0 => t('Select default profile'),
  );

  $profiles = MediaMosaCkConnectorWrapper::get_transcode_profiles();
  if ($profiles && isset($profiles->items->item)) {
    foreach ($profiles->items->item as $profile) {
      $options[(int) $profile->profile_id] = (string) $profile->profile;
    }
  }

  $form['mediamosa_ck_settings']['transcode_profile']['mediamosa_ck_transcode_profile_default'] = array(
    '#title' => t('Select your default transcode profile'),
    '#description' => t('The default transcode profile is used by the player, to determine the most preferred mediafile to view.'),
    '#type' => 'select',
    '#options' => $options,
    '#size' => 1,
    '#default_value' => MediaMosaCkConnectorWrapper::get_default_transcode_profile(),
  );

  // Remove none for the allowed list.
  unset($options[0]);
  $form['mediamosa_ck_settings']['transcode_profile']['mediamosa_ck_transcode_profiles_allowed'] = array(
    '#title' => t('Select your allowed transcode profiles'),
    '#description' => t('By default, all transcodes are allowed.'),
    '#size' => 20,
    '#type' => 'select',
    '#options' => $options,
    '#multiple' => TRUE,
    '#default_value' => array_keys(MediaMosaCkConnectorWrapper::get_allowed_transcode_profiles()),
  );

  // Start automatically transcodes?
  $form['mediamosa_ck_settings']['mediamosa_ck_transcodes'] = array(
    '#type' => 'fieldset',
    '#description' => t('Select the transcode after any upload to MediaMosa in the application. This setting is used for basic upload and bulk upload.'),
    '#title' => t('Automatic transcodes'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 12,
  );

  $default_transcode_profile = variable_get('mediamosa_ck_transcode_profile_default');
  if (!isset($default_transcode_profile)) {
    $default_transcode_profile = MediaMosaCkConnectorWrapper::get_default_transcode_profile();
  }
  $available_transcode_profiles = variable_get('mediamosa_ck_transcode_profiles_allowed');

  $mediamosa_ck_transcodes = variable_get('mediamosa_ck_transcodes_autostart');
  if (!isset($mediamosa_ck_transcodes)) {
    $mediamosa_ck_transcodes = array($default_transcode_profile);
  }
  $form['mediamosa_ck_settings']['mediamosa_ck_transcodes']['mediamosa_ck_transcodes_autostart'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Automatically start transcodes:'),
    '#options' => $available_transcode_profiles,
    '#default_value' => $mediamosa_ck_transcodes,
    '#description' => 'The list is taken from the defined allowed profiles (see above)',
  );

  // Media view settings.
  $form['mediamosa_ck_settings']['media_view'] = array(
    '#type' => 'fieldset',
    '#title' => 'View options',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Setting page for view options.'),
  );

  $form['mediamosa_ck_settings']['media_view']['mediamosa_ck_view_https'] = array(
    '#title' => t('Force Mediamosa backend play/view requests to use https:// instead of http://.'),
    '#description' => t('A view request to the backend will get a view object with links to media. Depending on the backend, this is served with http or https links. This option rewrites received http:// links with https://.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('mediamosa_ck_view_https', FALSE),
  );
  return $form;
}

/**
 * Implements hook_form_validate().
 *
 * Form validation handler for main configuration form for MediaMosa
 * Construction Kit.
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
 */
function mediamosa_ck_configuration_form_submit($form, &$form_state) {

  // Call our (optional) submit hooks.
  module_invoke_all('mediamosa_ck_configuration_collect_submit', $form, $form_state);
}
