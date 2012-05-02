<?php
/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Allows the profile to alter the site configuration form.
 */
function mediamosa_ck_profile_form_install_configure_form_alter(&$form, $form_state) {
  // Pre-populate the site name with the server name.
  $form['site_information']['site_name']['#default_value'] = $_SERVER['SERVER_NAME'];
}

/**
 * Implements hook_install_tasks().
 */
function mediamosa_ck_profile_install_tasks() {
  $tasks = array(
    'mediamosa_ck_profile_connector_form' => array(
      'display_name' => st('Setting up MediaMosa Connector'),
      'type' => 'form',
    ),

    '_mediamosa_ck_profile_setup_wysiwyg' => array(
      'run' => INSTALL_TASK_RUN_IF_REACHED,
    ),

    '_mediamosa_ck_profile_setup_intro_page' => array(
      'run' => INSTALL_TASK_RUN_IF_REACHED,
    ),
  );
  return $tasks;
}

/**
 * Setting up form mediamosa connector.
 */
function mediamosa_ck_profile_connector_form() {
  $form['connection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Connection settings to MediaMosa REST interface'),
    '#description' => t("Enter MediaMosa Application login and REST interface URL. Leave empty if you do not know the client application login. You can create a login in MediaMosa admin; Login as admin and goto MediaMosa->Configuration->Client applications and press on the tab 'add'. You can skip this step and setup the connector later."),
    '#collapsible' => TRUE,
    '#weight' => -5,
  );
  $form['connection']['mediamosa_connector_url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#description' => t('Enter the URL of the REST interface you want to connect to.'),
    '#required' => FALSE,
    '#default_value' => variable_get('mediamosa_connector_url', ''),
  );
  $form['connection']['mediamosa_connector_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('Enter the MediaMosa user name that needs to connect.'),
    '#required' => FALSE,
    '#default_value' => variable_get('mediamosa_connector_username', ''),
  );
  $form['connection']['mediamosa_connector_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#description' => t("Password might be required to login, is also called 'shared key'."),
    '#required' => FALSE,
    '#default_value' => variable_get('mediamosa_connector_password', ''),
  );

  $form['other']['check_connection'] = array(
    '#type' => 'checkbox',
    '#title' => t('Check the connection before saving my connection settings.'),
    '#description' => t('Enable this checkbox to verify the entered login. If will return to this form when login failed.'),
    '#default_value' => TRUE,
  );

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  return $form;
}

/**
 * Validation of mediamosa connector form.
 */
function mediamosa_ck_profile_connector_form_validate(&$form_state, $form) {

  // Values.
  $values = $form['values'];

  if (!empty($values['mediamosa_connector_url']) && !valid_url($values['mediamosa_connector_url'], TRUE)) {
    form_set_error('mediamosa_connector_url', t('The URL is not well formatted.'));
  }

  if (!empty($values['check_connection']) && (!empty($values['mediamosa_connector_url']) || !empty($values['mediamosa_connector_username']) || !empty($values['mediamosa_connector_password']))) {

    if (empty($values['mediamosa_connector_url'])) {
      form_set_error('mediamosa_connector_url', t('Value is required'));
    }
    if (empty($values['mediamosa_connector_username'])) {
      form_set_error('mediamosa_connector_username', t('Value is required'));
    }
    if (empty($values['mediamosa_connector_password'])) {
      form_set_error('mediamosa_connector_password', t('Value is required'));
    }

    // No errors? Then try to connect.
    if (!form_get_errors()) {
      if (!_mediamosa_connector_check_connection($values['mediamosa_connector_username'], $values['mediamosa_connector_password'], $values['mediamosa_connector_url'])) {
        form_set_error('', t('Unable to login'));
        drupal_set_message(t('Login failed, please check your input'), 'error');
      }
      else {
        drupal_set_message(t('Login successful, your MediaMosa connector has been setup.'));
      }
    }
  }
}

/**
 * Submit form mediamosa connector.
 */
function mediamosa_ck_profile_connector_form_submit($form, &$form_state) {

  variable_set('mediamosa_connector_url', $form_state['values']['mediamosa_connector_url']);
  variable_set('mediamosa_connector_username', $form_state['values']['mediamosa_connector_username']);
  variable_set('mediamosa_connector_password', $form_state['values']['mediamosa_connector_password']);
}

/**
 * Setup wysiwyg editor.
 */
function _mediamosa_ck_profile_setup_wysiwyg() {
  $format = 'full_html';
  $editor = 'ckeditor';
  $settings = 'a:20:{s:7:"default";i:1;s:11:"user_choose";i:0;s:11:"show_toggle";i:1;s:5:"theme";s:8:"advanced";s:8:"language";s:2:"en";s:7:"buttons";a:1:{s:6:"drupal";a:1:{s:5:"media";i:1;}}s:11:"toolbar_loc";s:3:"top";s:13:"toolbar_align";s:4:"left";s:8:"path_loc";s:6:"bottom";s:8:"resizing";i:1;s:11:"verify_html";i:1;s:12:"preformatted";i:0;s:22:"convert_fonts_to_spans";i:1;s:17:"remove_linebreaks";i:1;s:23:"apply_source_formatting";i:0;s:27:"paste_auto_cleanup_on_paste";i:0;s:13:"block_formats";s:32:"p,address,pre,h2,h3,h4,h5,h6,div";s:11:"css_setting";s:5:"theme";s:8:"css_path";s:0:"";s:11:"css_classes";s:0:"";}';

  db_query("INSERT INTO {wysiwyg} SET editor = :editor, settings = :settings, format = :format", array(':editor' => $editor, ':settings' => $settings, ':format' => $format));
}

/**
 * Create into page.
 */
function _mediamosa_ck_profile_setup_intro_page() {
  // Create basic page for explaining the user what to do...
  $node = new stdClass();
  $node->uid = 1; // Admin
  $node->status = 1; // Published
  $node->promote = 1; // On front page.
  $node->type = 'page';
  $node->locked = 0;
  $node->has_title = 1;
  $node->has_body = 1;
  $node->title_label = 'Title';
  $node->body_label = 'Body';
  $node->custom = 1;
  $node->language = LANGUAGE_NONE;

  $node->title = st('Introducing MediaMosa Construction Kit');

  $text = file_get_contents(dirname(__FILE__) . '/mediamosa_ck_profile.page.txt');
  $node->body[$node->language][0]['value'] = st(
    $text,
    array(
      '!url_mediamosa_connector' => url('admin/config/media/mediamosa/connector', array('absolute' => TRUE)),
      '!url_mmck_settings' => url('admin/config/media/mediamosa_ck/config', array('absolute' => TRUE)),
      '!url_wysiwyg' => url('admin/config/content/wysiwyg', array('absolute' => TRUE)),
    )
  );

  // Save it.
  node_save($node);
}
