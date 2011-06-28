<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 *  @file
 *  Embedded Video Field provider file for MediaMosa.org helpers.
 */

define('SUBMIT_TEXT_PREFS_REMOVE_ACCESS_RULE', '<< Remove access rule');
define('SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM', 'Add domain / realm >>');

/**
 * Form alter helper for searching cck field.
 */
function _media_mediamosa_find_cck(&$form, &$form_state, $key, $element) {
  if (drupal_substr($key, 0, drupal_strlen('field_')) == 'field_') {
    $i = 0;
    while (isset($element[$i])) {
      if (isset($element[$i]['embed']['#description'])) {
        // Search 'mediamosa'.
        if (strpos($element[$i]['embed']['#description'], 'mediamosa') !== FALSE) {
          _media_mediamosa_change_cck($form, $form_state, $key, $element, $i);
        }
      }
      $i++;
    }
  }
}

/**
 * Form alter helper for changing cck field.
 */
function _media_mediamosa_change_cck(&$form, &$form_state, $key, $element, $i) {
  global $base_url;

  // Get the asset_id.
  $asset_id = $form[$key][$i]['embed']['#default_value'];
  $mediafile_old = _media_mediamosa_get_mediafile_original($asset_id);

  // Our ahah.js.
  drupal_add_js(drupal_get_path('module', 'media_mediamosa') . '/js/ahah.js', 'module', 'footer');

  // Create the upload ticket.
  $ticket = _media_mediamosa_upload_prepare($asset_id);

  if (!is_array($ticket)) {
    drupal_set_message(t('Error creating mediafile upload ticket.'), 'error');
    watchdog('media_mediamosa', t('Error creating mediafile upload ticket.'), array(), WATCHDOG_ERROR);
    // Go to the front page.
    drupal_goto('');
    return FALSE;
  }

  // MediaMosa.
  $form['mediamosa'] = array(
    '#type' => 'value',
    '#value' => 'TRUE',
  );
  // Asset id.
  $form['asset_id'] = array(
    '#type' => 'value',
    '#value' => $ticket['asset_id'],
  );
  // Mediafile id.
  $form['mediafile_id'] = array(
    '#type' => 'value',
    '#value' => $ticket['mediafile_id'],
  );
  // Mediafile id old.
  $form['mediafile_id_old'] = array(
    '#type' => 'value',
    '#value' => $mediafile_old['mediafile_id'],
  );

  $form[$key][$i]['embed']['#type'] = 'value';
  // Replace the text.
  $form[$key][$i]['embed']['#description'] = str_replace('Enter the URL or Embed Code here.', t('Enter the mediafile id - in case of MediaMosa -, or URL or Embed Code in other cases here. You can leave it empty, if you choose a file to uplad (see the file upload near here).'), $element[$i]['embed']['#description']);

  // Implement the file upload fieldset.
  $form['mediamosa_file'] = array(
    '#type' => 'fieldset',
    '#title' => t('File'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#weight' => $form[$key]['#weight'],
  );

  // Still default.
  $still_default = '';
  $options_helper = array();
  $options = array();
  if (isset($mediafile_old['still'])) {

    // Fix the problem with 1 or 2+ items in array.
    $tmp = isset($mediafile_old['still'][0]) ? $mediafile_old['still'] : array(0 => $mediafile_old['still']);
    unset($mediafile_old['still']);
    $mediafile_old['still'] = $tmp;

    // Get the stills.
    foreach ($mediafile_old['still'] as $still) {
      // Add the still delete checkbox here.
      $form['still_delete_' . $still['mediafile_id']] = array(
        '#type' => 'checkbox',
        '#title' => t('Check this, if you want to delete this still.'),
        '#default_value' => 0,
      );

      // Radio button options.
      // Key is like: '0000000004_eb7NQPY7YeQWYRK2SDfsSMCU'. This is good for ordering.
      $options_helper[sprintf('%010s', (int) $still['still_time_code']) . '_' . $still['mediafile_id']] = array(
        'text' =>
          theme_image((string) $still['still_ticket'], t('Still image'), t('Still image'), NULL, FALSE) .
          '<div id="still_time">' . t('Time @time sec', array('@time' => gmdate("H:i:s", $still['still_time_code']),)) . '</div>' .
          '<div id="still_size">' . t('Size @widthx@height px', array('@width' => $still['width'], '@height' => $still['height'],)) . '</div>' .
          '<div id="filesize">' . t('File size @filesize KB', array('@filesize' => $still['filesize']>>10,)) . '</div>',
        'mediafile_id' => (string) $still['mediafile_id']
      );

      if ($still['still_default'] == 'TRUE') {
        $still_default = (string) $still['mediafile_id'];
      }
    }

    // Order them by time.
    ksort($options_helper);

    // Create the options array.
    foreach ($options_helper as $options_helper_key => $options_helper_value) {
      $options[$options_helper_value['mediafile_id']] = $options_helper_value['text'];
    }

    // Make the form element.
    $form['still_images'] = array(
      '#type' => 'radios',
      '#title' => t('Set still image to default'),
      '#default_value' => $still_default,
      '#options' => $options,
    );

    // Special theme for still images, for inserting the still checkboxes to the still radio button.
    $form['#theme'] = 'media_mediamosa_node_form';
  }

  // Still generate.
  if (variable_get('media_mediamosa_still', 0) && user_access('access still settings on node page')) {
    _media_mediamosa_still_create_form($form, $form_state, array(), $mediafile_old['mediafile_id']);
  }

  // Transcoded mediafile generate.
  if (variable_get('media_mediamosa_transcode_profile', 0) && user_access('access transcode settings on node page')) {
    _media_mediamosa_transcode_profile_create_form($form, $form_state, $mediafile_old['mediafile_id']);
  }

  // Access restrictions.
  if (variable_get('media_mediamosa_access_control', 0) && user_access('access ACL settings on node page')) {
    _media_mediamosa_access_create_form($form, $form_state, $mediafile_old['mediafile_id']);
  }

  // Change position of delete checkbox.
  if ($form[$key][$i]['emvideo']['delete']) {
    $form['mediamosa_file'][$key][$i]['emvideo']['delete'] = $form[$key][$i]['emvideo']['delete'];
    $form['mediamosa_file'][$key][$i]['emvideo']['delete']['#description'] = t('Checking this field will delete all of the videos in this node. You can not use this, if you are attaching a media file in the same time.');
    unset($form[$key][$i]['emvideo']['delete']);
  }

  // File button.
  $form['mediamosa_file']['mediamosa_file_upload'] = array(
    '#name' => 'file',
    '#title' => t('Attach a media file to upload MediaMosa'),
    '#description' => t('If you upload a file to MediaMosa here, it will replace the original media file (if there is any). If you do not upload a file, you may change the other fields of this node without changing the original media file.'),
    '#type' => 'file',
    '#prefix' => '<div id="upload-file-wrapper">',
  );
  $form['mediamosa_file']['upload_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Attach'),
    '#submit' => array('not_yet_implemented'), //none JS version.
    '#ahah' => array(
      'event' => 'click',
      'path' => $ticket['action'],
      'wrapper' => 'upload-file-wrapper',
      'method' => 'replace',
      'effect' => 'fade',
      // TODO: Fix the progress bar.
      //'progress' => array(
      //  'type' => 'bar',
      //  'url' => $base_url . '/' . drupal_get_path('module', 'media_mediamosa') . '/json.php?url=' . urlencode($ticket['uploadprogress_url'] . '?id=' . urlencode($upload_progress_key)),
      //),
    ),
    '#suffix' => '</div>',
  );
  // Success.
  $form['mediamosa_file']['mediamosa_success'] = array(
    '#type' => 'hidden',
    '#default_value' => (empty($form_state['values']) ? 'FALSE' : $form_state['values']['mediamosa_success']),
  );
  // Was it uploaded already (is it a preview page)?
  $form['mediamosa_file']['hide'] = array(
    '#value' => sprintf('<script type="text/javascript">
<!--//--><![CDATA[//><!--

if (document.getElementById("edit-mediamosa-success").value == "TRUE") {
  $("#upload-file-wrapper").text("%s");
}

//--><!]]>
</script>',
    MEDIA_MEDIAMOSA_UPLOAD_TEXT
    ),
  );


  // Additional form elements.
  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['#submit'][] = '_media_mediamosa_file_upload_submit';

  unset($form[$key][$i]['value_markup']['#value']);
}

/**
 * Node form theme.
 */
function theme_media_mediamosa_node_form($form) {
  $output = '';

  // Insert the still checkboxes into still radio buttons.
  if (!empty($form['still_images']['#options'])) {
    foreach ($form['still_images']['#options'] as $radio_key => $radio_value) {
      if (!empty($form['still_delete_' . $radio_key])) {
        $output_checkbox = drupal_render($form['still_delete_' . $radio_key]);
        if (!empty($form['still_images']['#options'][$radio_key]['#title'])) {
          $form['still_images'][$radio_key]['#title'] .= $output_checkbox;
        }
      }
    }
  }

  // Buttons should be the last.
  $buttons = drupal_render($form['buttons']);

  $output .= drupal_render($form) . $buttons;

  return $output;
}

/**
 * Still generation form.
 *
 * @param array $form
 * @param array $form_state
 * @param array $options
 */
function _media_mediamosa_still_create_form(&$form, &$form_state, $options = array(), $mediafile_id_old = NULL) {
  // Node type.
  $node_type = '_' . $form['type']['#value'];

  // JS.
  drupal_add_js(drupal_get_path('module', 'media_mediamosa') . '/js/still.js');

  // Get possible values.
  $values = empty($form_state['values']) ? array() : $form_state['values'];

  $options += array(
    'hidden' => array(),
  );

  $form['mediamosa_file']['still'] = array(
    '#type' => 'fieldset',
    '#title' => 'Still generate parameters',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['mediamosa_file']['still']['still_type'] = array(
    '#title' => t('Select your still type'),
    '#type' => 'select',
    '#options' => array(
      MEDIA_MEDIAMOSA_STILL_TYPE_NONE => 'Default',
      MEDIA_MEDIAMOSA_STILL_TYPE_NORMAL => '# of stills per mediafile',
      MEDIA_MEDIAMOSA_STILL_TYPE_SECOND => 'A still every # of seconds',
      MEDIA_MEDIAMOSA_STILL_TYPE_SCENE => 'A still for each scene',
    ),
    '#size' => 1,
    '#required' => TRUE,
    '#default_value' => (empty($values['still_type']) ? variable_get('media_mediamosa_still_type' . $node_type, variable_get('media_mediamosa_still_type', MEDIA_MEDIAMOSA_STILL_TYPE_NONE)) : $values['still_type']),
    '#attributes' => array('onchange' => "media_mediamosa_still_hide_by_type(this);"),
  );

  // All
  $form['mediamosa_file']['still']['size'] = array(
    '#title' => t('Size of still (in pixels, WidthxHeight)'),
    '#type' => 'textfield',
    '#size' => 15,
    '#default_value' => (empty($values['size']) ? variable_get('media_mediamosa_still_size' . $node_type, variable_get('media_mediamosa_still_size', MEDIA_MEDIAMOSA_STILL_SIZE)) : $values['size']),
    '#required' => TRUE,
  );

  // All
  $form['mediamosa_file']['still']['h_padding'] = array(
    '#title' => t('Horizontal padding (in pixels)'),
    '#type' => 'textfield',
    '#size' => 4,
    '#default_value' => (empty($values['h_padding']) ? variable_get('media_mediamosa_still_h_padding' . $node_type, variable_get('media_mediamosa_still_h_padding', MEDIA_MEDIAMOSA_STILL_H_PADDING)) : $values['h_padding']),
    '#required' => TRUE,
    '#description' => t('You can use it, if you want add horizontal padding (black strips) to your still images'),
  );

  // All
  $form['mediamosa_file']['still']['v_padding'] = array(
    '#title' => t('Vertical padding (in pixels)'),
    '#type' => 'textfield',
    '#size' => 4,
    '#default_value' => (empty($values['v_padding']) ? variable_get('media_mediamosa_still_v_padding' . $node_type, variable_get('media_mediamosa_still_v_padding', MEDIA_MEDIAMOSA_STILL_V_PADDING)) : $values['v_padding']),
    '#required' => TRUE,
    '#description' => t('You can use it, if you want add vertical padding (black strips) to your still images'),
  );

  // NONE.
  $form['mediamosa_file']['still']['still_time_code'] = array(
    '#prefix' => '<div id="show-media_mediamosa-still-time-code" style="display:' . (!$still_type || $still_type == MEDIA_MEDIAMOSA_STILL_TYPE_NONE ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('Still time code'),
    '#type' => 'textfield',
    '#size' => 3,
    '#default_value' => (empty($values['still_time_code']) ? variable_get('media_mediamosa_still_time_code' . $node_type, variable_get('media_mediamosa_still_time_code', MEDIA_MEDIAMOSA_STILL_TIME_CODE)) : $values['still_time_code']),
    '#description' => t('You can define from which second the still come from'),
  );

  // NORMAL.
  $form['mediamosa_file']['still']['still_per_mediafile'] = array(
    '#prefix' => '<div id="show-media_mediamosa-still-per-mediafile" style="display:' . ($still_type == MEDIA_MEDIAMOSA_STILL_TYPE_NORMAL ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('# of stills per mediafile'),
    '#type' => 'textfield',
    '#size' => 3,
    '#default_value' => (empty($values['still_per_mediafile']) ? variable_get('media_mediamosa_still_per_mediafile' . $node_type, variable_get('media_mediamosa_still_per_mediafile', MEDIA_MEDIAMOSA_STILL_PER_MEDIAFILE)) : $values['still_per_mediafile']),
    '#description' => t('You can define here how many still images you would like to create from the video file'),
  );

  // SECOND.
  $form['mediamosa_file']['still']['still_every_second'] = array(
    '#prefix' => '<div id="show-media_mediamosa-still-every-second" style="display:' . ($still_type == MEDIA_MEDIAMOSA_STILL_TYPE_SECOND ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('One still every # of seconds'),
    '#type' => 'textfield',
    '#size' => 6,
    '#default_value' => (empty($values['still_every_second']) ? variable_get('media_mediamosa_still_every_second' . $node_type, variable_get('media_mediamosa_still_every_second', MEDIA_MEDIAMOSA_STILL_EVERY_SECOND)) : $values['still_every_second']),
    '#description' => t('You can define here how many seconds should be between two created images according your video duration'),
  );

  // NORMAL / SECOND.
  $form['mediamosa_file']['still']['start_time'] = array(
    '#prefix' => '<div id="show-media_mediamosa-start-time" style="display:' . ($still_type == MEDIA_MEDIAMOSA_STILL_TYPE_NORMAL || $still_type == MEDIA_MEDIAMOSA_STILL_TYPE_SECOND ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('Start time in seconds'),
    '#type' => 'textfield',
    '#size' => 7,
    '#default_value' => (empty($values['start_time']) ? variable_get('media_mediamosa_still_start_time' . $node_type, variable_get('media_mediamosa_still_start_time', MEDIA_MEDIAMOSA_STILL_START_TIME)) : $values['start_time']),
    '#description' => t('You may define a starting point to your still generation according to your video duration'),
  );

  // NORMAL / SECOND.
  $form['mediamosa_file']['still']['end_time'] = array(
    '#prefix' => '<div id="show-media_mediamosa-end-time" style="display:' . ($still_type == MEDIA_MEDIAMOSA_STILL_TYPE_NORMAL || $still_type == MEDIA_MEDIAMOSA_STILL_TYPE_SECOND ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('End time in seconds'),
    '#type' => 'textfield',
    '#size' => 7,
    '#default_value' => (empty($values['end_time']) ? variable_get('media_mediamosa_still_end_time' . $node_type, variable_get('media_mediamosa_still_end_time', MEDIA_MEDIAMOSA_STILL_END_TIME)) : $values['end_time']),
    '#description' => t('You may define an ending point to your still generation according to your video duration'),
  );

  // Save the contents of options.
  foreach ($options['hidden'] as $name => $value) {
    $form['hidden'][$name] = array(
      '#type' => 'hidden',
      '#value' => $value,
    );
  }

  if ($mediafile_id_old) {
    _media_mediamosa_get_running_jobs($form['mediamosa_file']['still'], $mediafile_id_old, MEDIA_MEDIAMOSA_JOB_TYPE_STILL);

    $form['mediamosa_file']['still']['still_for_old_mediafile'] = array(
      '#type' => 'submit',
      // Don't change the #value, without changing the submit check too.
      '#value' => t('Create stills for the current mediafile'),
    );
    $form['mediamosa_file']['still']['notice'] = array(
      '#type' => 'item',
      '#title' => t('Notice'),
      '#value' => t('If you click this button, your other values (eg. title) will not be saved.'),
    );
  }
}

/**
 * Transcoded mediafile generation form.
 *
 * @param array $form
 * @param array $form_state
 * @param array $options
 */
function _media_mediamosa_transcode_profile_create_form(&$form, &$form_state, $mediafile_id_old = NULL) {
  // Node type.
  $node_type = '_' . $form['type']['#value'];

  // Get possible values.
  $values = empty($form_state['values']) ? array() : $form_state['values'];

  $form['mediamosa_file']['transcode_profile'] = array(
    '#type' => 'fieldset',
    '#title' => 'Transcode profile parameter',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  // Get default transcode profile and the options.
  $profiles = _media_mediamosa_transcode_profiles_get();
  $options = array(
    'none' => t('---'),
  );
  $transcode_profile_default = MEDIA_MEDIAMOSA_TRANSCODE_PROFILE_DEFAULT;
  foreach ($profiles as $profile_id => $profile) {
    $options[$profile_id] = $profile['profile'];
    if ($profile['default'] == 'TRUE') {
      $transcode_profile_default = $profile_id;
    }
  }

  if (variable_get('media_mediamosa_transcode_profile_default', NULL)) {
    $transcode_profile_default = variable_get('media_mediamosa_transcode_profile_default', NULL);
  }
  if (variable_get('media_mediamosa_transcode_profile_default' . $node_type, NULL)) {
    $transcode_profile_default = variable_get('media_mediamosa_transcode_profile_default' . $node_type, NULL);
  }
  $transcode_profile_default = empty($values['transcode_profile_default']) ? $transcode_profile_default : $values['transcode_profile_default'];

  // Default profile form.
  $form['mediamosa_file']['transcode_profile']['transcode_profile_default'] = array(
    '#title' => t('Select your default transcode profile'),
    '#type' => 'select',
    '#options' => variable_get('media_mediamosa_transcode_profile_default_options_value', $options),
    '#size' => 1,
    '#required' => TRUE,
    '#default_value' => $transcode_profile_default,
  );

  if ($mediafile_id_old) {
    _media_mediamosa_get_running_jobs($form['mediamosa_file']['transcode_profile'], $mediafile_id_old, MEDIA_MEDIAMOSA_JOB_TYPE_TRANSCODE);

    $form['mediamosa_file']['transcode_profile']['transcode_profile_for_old_mediafile'] = array(
      '#type' => 'submit',
      // Don't change the #value, without changing the submit check too.
      '#value' => t('Transcode the current mediafile'),
    );
    $form['mediamosa_file']['transcode_profile']['notice'] = array(
      '#type' => 'item',
      '#title' => t('Notice'),
      '#value' => t('If you click this button, your other values (eg. title) will not be saved.'),
    );
  }
}

/**
 * Access rules generation form.
 *
 * @param array $form
 * @param array $form_state
 */
function _media_mediamosa_access_create_form(&$form, &$form_state, $mediafile_id_old) {
  // No default sitewide settings, all default settings are related to a particular node type.
  $node_type = '_' . $form['type']['#value'];

  $form['mediamosa_file']['access'] = array(
    '#type' => 'fieldset',
    '#title' => 'Access',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  // No rules defined.
  $rules = array();

  if (isset($form['nid']['#value'])) {
    // We are in edit mode.
    //$mediafile_id = $form['mediafile_id']['#value'];
    $mediafile_id = $mediafile_id_old;

    if ($mediafile_id) {
      $mediafile = _media_mediamosa_mediafile_get($mediafile_id);
      $user_id = (string) $mediafile['owner_id'];
      $rules_encoded = _media_mediamosa_get_access_control($mediafile_id, $user_id);
    }
  }

  if ($rules_encoded) {
    // Did we get something? Lets apply it!
    foreach ($rules_encoded as $item) {
      if (isset($item['aut_realm'])) {
        $rules[] = $item['aut_realm'];
      }
      elseif (isset($item['aut_domain'])) {
        $rules[] = $item['aut_domain'];
      }
    }
  }

  if (!$rules) {
    $rules = variable_get('media_mediamosa_default_domains_and_realms_access' . $node_type, array());
  }
  sort($rules);

  $form['mediamosa_file']['access']['domain_realm'] = array(
    '#type' => 'textarea',
    '#title' => t('Authorized domains and realms'),
    '#description' => t('To restrict access please enter allowed domains and realms. Each line contrains realm or domain.<br />Examples: foo.com, @bar.com'),
    '#default_value' => implode("\n", $rules),
  );
}

/**
 * Still settings form.
 *
 * @param $node_type
 *
 * @return $form
 */
function _media_mediamosa_still_settings_form($node_type = '') {
  $form['still'] = array(
    '#type' => 'fieldset',
    '#title' => 'Still generate parameters',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  if ($node_type) {
    $node_type = '_' . $node_type;

    $form['still']['media_mediamosa_still_local' . $node_type] = array(
      '#type' => 'checkbox',
      '#title' => t('Use this settings instead of the global still settings'),
      '#default_value' => variable_get('media_mediamosa_still_local' . $node_type, 0),
    );
  }

  $form['still']['media_mediamosa_still_type' . $node_type] = array(
    '#title' => t('Select your still type'),
    '#type' => 'select',
    '#options' => array(
      MEDIA_MEDIAMOSA_STILL_TYPE_NONE => 'Default',
      MEDIA_MEDIAMOSA_STILL_TYPE_NORMAL => '# of stills per mediafile',
      MEDIA_MEDIAMOSA_STILL_TYPE_SECOND => 'A still every # of seconds',
      MEDIA_MEDIAMOSA_STILL_TYPE_SCENE => 'A still for each scene',
    ),
    '#size' => 1,
    '#default_value' => variable_get('media_mediamosa_still_type' . $node_type, variable_get('media_mediamosa_still_type', MEDIA_MEDIAMOSA_STILL_TYPE_NONE)),
  );

  $form['still']['media_mediamosa_still_size' . $node_type] = array(
    '#title' => t('Size of still (in pixels, WidthxHeight)'),
    '#type' => 'textfield',
    '#size' => 15,
    '#default_value' => variable_get('media_mediamosa_still_size' . $node_type, variable_get('media_mediamosa_still_size', MEDIA_MEDIAMOSA_STILL_SIZE)),
  );

  $form['still']['media_mediamosa_still_h_padding' . $node_type] = array(
    '#title' => t('Horizontal padding (in pixels)'),
    '#type' => 'textfield',
    '#size' => 4,
    '#default_value' => variable_get('media_mediamosa_still_h_padding' . $node_type, variable_get('media_mediamosa_still_h_padding', MEDIA_MEDIAMOSA_STILL_H_PADDING)),
    '#description' => t('You can use it, if you want add horizontal padding (black strips) to your still images'),
  );

  $form['still']['media_mediamosa_still_v_padding' . $node_type] = array(
    '#title' => t('Vertical padding (in pixels)'),
    '#type' => 'textfield',
    '#size' => 4,
    '#default_value' => variable_get('media_mediamosa_still_v_padding' . $node_type, variable_get('media_mediamosa_still_v_padding', MEDIA_MEDIAMOSA_STILL_V_PADDING)),
    '#description' => t('You can use it, if you want add vertical padding (black strips) to your still images'),
  );

  $form['still']['media_mediamosa_still_time_code' . $node_type] = array(
    '#title' => t('Still time code'),
    '#type' => 'textfield',
    '#size' => 3,
    '#default_value' => variable_get('media_mediamosa_still_time_code' . $node_type, variable_get('media_mediamosa_still_time_code', MEDIA_MEDIAMOSA_STILL_TIME_CODE)),
    '#description' => t('You can define from which second the still come from'),
  );

  $form['still']['media_mediamosa_still_per_mediafile' . $node_type] = array(
    '#title' => t('# of stills per mediafile'),
    '#type' => 'textfield',
    '#size' => 3,
    '#default_value' => variable_get('media_mediamosa_still_per_mediafile' . $node_type, variable_get('media_mediamosa_still_per_mediafile', MEDIA_MEDIAMOSA_STILL_PER_MEDIAFILE)),
    '#description' => t('You can define here how many still images you would like to create from the video file'),
  );

  $form['still']['media_mediamosa_still_every_second' . $node_type] = array(
    '#title' => t('One still every # of seconds'),
    '#type' => 'textfield',
    '#size' => 6,
    '#default_value' => variable_get('media_mediamosa_still_every_second' . $node_type, variable_get('media_mediamosa_still_every_second', MEDIA_MEDIAMOSA_STILL_EVERY_SECOND)),
    '#description' => t('You can define here how many seconds should be between two created images according your video duration'),
  );

  $form['still']['media_mediamosa_still_start_time' . $node_type] = array(
    '#title' => t('Start time in seconds'),
    '#type' => 'textfield',
    '#size' => 7,
    '#default_value' => variable_get('media_mediamosa_still_start_time' . $node_type, variable_get('media_mediamosa_still_start_time', MEDIA_MEDIAMOSA_STILL_START_TIME)),
    '#description' => t('You may define a starting point to your still generation according to your video duration'),
  );

  $form['still']['media_mediamosa_still_end_time' . $node_type] = array(
    '#title' => t('End time in seconds'),
    '#type' => 'textfield',
    '#size' => 7,
    '#default_value' => variable_get('media_mediamosa_still_end_time' . $node_type, variable_get('media_mediamosa_still_end_time', MEDIA_MEDIAMOSA_STILL_END_TIME)),
    '#description' => t('You may define an ending point to your still generation according to your video duration'),
  );

  return $form;
}

/**
 * Transcode profile settings form.
 *
 * @param $node_type
 *
 * @return $form
 */
function _media_mediamosa_transcode_profile_settings_form($node_type = '') {
  $form['transcode_profile'] = array(
    '#type' => 'fieldset',
    '#title' => 'Transcode profile parameter',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  if ($node_type) {
    $node_type = '_' . $node_type;

    $form['transcode_profile']['media_mediamosa_transcode_profile_local' . $node_type] = array(
      '#type' => 'checkbox',
      '#title' => t('Use this settings instead of the global transcode profile settings'),
      '#default_value' => variable_get('media_mediamosa_transcode_profile_local' . $node_type, 0),
    );
  }


  $profiles = _media_mediamosa_transcode_profiles_get();
  $options = array(
    'none' => t('---'),
  );
  $transcode_profile_default = MEDIA_MEDIAMOSA_TRANSCODE_PROFILE_DEFAULT;
  if (is_array($profiles)) {
    foreach ($profiles as $profile_id => $profile) {
      $options[$profile_id] = $profile['profile'];
      if ($profile['default'] == 'TRUE') {
        $transcode_profile_default = $profile_id;
      }
    }
  }

  $form['transcode_profile']['media_mediamosa_transcode_profile_default' . $node_type] = array(
    '#title' => t('Select your default transcode profile'),
    '#type' => 'select',
    '#options' => variable_get('media_mediamosa_transcode_profile_default_options_value', $options),
    '#size' => 1,
    '#default_value' => variable_get('media_mediamosa_transcode_profile_default' . $node_type, $transcode_profile_default),
  );

  if (!$node_type) {
    $form['transcode_profile']['media_mediamosa_transcode_profile_default_options'] = array(
      '#title' => t('Select your default transcode profile'),
      '#description' => t('If you do not choose one, you can not transcode mediafiles.'),
      '#type' => 'select',
      '#options' => $options,
      '#size' => 1,
      '#multiple' => TRUE,
      '#default_value' => variable_get('media_mediamosa_transcode_profile_default_options', array_keys($options)),
    );
  }

  return $form;
}

/**
 * Access settings form.
 *
 * @param $node_type
 *
 * @return $form
 */
function _media_mediamosa_access_settings_form($node_type = '') {
  $form['access'] = array(
    '#type' => 'fieldset',
    '#title' => 'Access',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  if ($node_type) {
    $node_type = '_' . $node_type;
  }

  $rules = variable_get('media_mediamosa_default_domains_and_realms_access' . $node_type, array());

  $form['access']['domain_realm'] = array(
    '#type' => 'textarea',
    '#title' => t('Authorized domains and realms'),
    '#description' => t('To restrict access please enter allowed domains and realms. Each line contrains realm or domain.<br />Examples: foo.com, @bar.com'),
    '#default_value' => implode("\n", $rules),
  );

  return $form;
}

/**
 * Validate function for access form.
 */
function _media_mediamosa_access_settings_form_validate($form, $form_state) {

  // We are here only if the field is accessible and editable by user.
  $value = $form_state['values']['domain_realm'];

  preg_match_all("/[^\s]+/", $value, $match);
  $rules = $match[0];

  $domains = preg_replace("/.*@/", "", $rules);

  foreach ($domains as $domain) {
    // No reason to check for empty string here.
    // However, we can check even for domain presence here, if needed.
    if (!checkdnsrr($domain, "ANY")) {
      drupal_set_message(t('Seems that you mentioned an invalid domain %name .', array('%name' => $domain)), 'warning');
    }
  }
}

/**
 * Menu hook.
 */
function _media_mediamosa_settings() {
  $form = array();

  // Basic configuration.

  $form['media_mediamosa_conf'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload to Mediamosa'),
  );

  $form['media_mediamosa_conf']['media_mediamosa_still'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create still images'),
    '#description' => t('If you check this, MediaMosa will create stills after upload the media file.'),
    '#default_value' => variable_get('media_mediamosa_still', 0),
  );

  $form['media_mediamosa_conf']['media_mediamosa_transcode_profile'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create a transcoded mediafile'),
    '#description' => t('If you check this, MediaMosa will create transcoded mediafile after upload the video file.'),
    '#default_value' => variable_get('media_mediamosa_transcode_profile', 0),
  );

  $form['media_mediamosa_conf']['media_mediamosa_access_control'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide access control to mediafiles'),
    '#description' => t('If you check this, MediaMosa will provide an ability to set access restrictions per content type and per node.'),
    '#default_value' => variable_get('media_mediamosa_access_control', 0),
  );

  // Still.
  $form = array_merge($form, _media_mediamosa_still_settings_form());

  // Profiles.
  $form = array_merge($form, _media_mediamosa_transcode_profile_settings_form());

  // Submit.
  $form['#submit'][] = '_media_mediamosa_still_settings_submit';
  $form['#submit'][] = '_media_mediamosa_transcode_profile_settings_submit';

  return system_settings_form($form);
}

/**
 * Node form submit.
 *
 * @param string $asset_id
 */
function _media_mediamosa_upload_prepare($asset_id = NULL) {
  $user_id = _media_mediamosa_get_user_id();

  // REST calls.

  if ($asset_id) {
    // Delete empty mediafiles (trash from earlier).
    $asset = _media_mediamosa_responsexml2array(_media_mediamosa_asset_get($asset_id));

    if ($asset['mediafiles']['mediafile']) {

      // Fix the problem with 1 or 2+ items in array.
      $tmp = isset($asset['mediafiles']['mediafile'][0]) ? $asset['mediafiles']['mediafile'] : array(0 => $asset['mediafiles']['mediafile']);
      unset($asset['mediafiles']['mediafile']);
      $asset['mediafiles']['mediafile'] = $tmp;

      // Search the oldest original mediafile.
      $oldest_original_mediafile = NULL;
      foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
        if ($mediafile['is_original_file'] == 'TRUE') {
          // Original mediafiles.
          if (!$oldest_original_mediafile || $mediafile['created'] < $oldest_original_mediafile['created']) {
            // Oldest original mediafile.
            $oldest_original_mediafile = $mediafile;
          }
        }
      }

      foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
        if (!$mediafile['metadata']['file_duration'] ||
            ($mediafile['mediafile_id'] != $oldest_original_mediafile['mediafile_id'] && $mediafile['mediafile_id_source'] != $oldest_original_mediafile['mediafile_id'])) {
          // Empty mediafile. And not the oldest original and transcoded mediafile.
          _media_mediamosa_mediafile_delete($mediafile['mediafile_id'], $mediafile['owner_id']);
        }
      }
    }
  }
  else {
    // Create an assset.
    $asset_id = _media_mediamosa_asset_create($user_id);

    if (!$asset_id) {
      return FALSE;
    }
  }

  // Create a mediafile.
  $mediafile_id = _media_mediamosa_mediafile_create($asset_id, $user_id);
  if (!$mediafile_id) {
    return FALSE;
  }

  // Create a ticket for upload.
  $ticket = _media_mediamosa_uploadticket_create($asset_id, $mediafile_id, $user_id);
  if (!$ticket) {
    return FALSE;
  }

  $ticket['action'] .=  '&user_id=' . urlencode($user_id);

  $ticket['asset_id'] = $asset_id;
  $ticket['mediafile_id'] = $mediafile_id;

  return $ticket;
}

/**
 * Get user_id.
 *
 * @return $user_id
 */
function _media_mediamosa_get_user_id() {
  global $user;
  // User details.
  $user_id = $user->name;

  if (!$user_id) {
    $user_id = 'anonymous';
  }

  return $user_id;
}

/**
 * Get the original mediafile of an asset.
 *
 * @param string $asset_id.
 */
function _media_mediamosa_get_mediafile_original($asset_id) {
  $asset = _media_mediamosa_asset_get($asset_id);

  if (!empty($asset['mediafiles']['mediafile'])) {

    // Fix the problem with 1 or 2+ items in array.
    $tmp = isset($asset['mediafiles']['mediafile'][0]) ? $asset['mediafiles']['mediafile'] : array(0 => $asset['mediafiles']['mediafile']);
    unset($asset['mediafiles']['mediafile']);
    $asset['mediafiles']['mediafile'] = $tmp;

    // Search the oldest original mediafile.
    $oldest_original_mediafile = NULL;
    foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
      if ($mediafile['is_original_file'] == 'TRUE' && !empty($mediafile['metadata']['file_duration'])) {
        // Original mediafiles.
        if (!$oldest_original_mediafile || $mediafile['created'] < $oldest_original_mediafile['created']) {
          // Oldest original mediafile.
          $oldest_original_mediafile = $mediafile;
        }
      }
    }
    return $oldest_original_mediafile;
  }

  return FALSE;
}

/**
 * Delete the transcoded mediafile of an asset.
 *
 * @param string $asset_id.
 *
 * @return array $mediafile_ids
 */
function _media_mediamosa_delete_mediafile_transcoded($asset_id) {
  $mediafile_ids = array();

  $asset = _media_mediamosa_responsexml2array(_media_mediamosa_asset_get($asset_id));

  if ($asset['mediafiles']['mediafile']) {

    // Fix the problem with 1 or 2+ items in array.
    $tmp = isset($asset['mediafiles']['mediafile'][0]) ? $asset['mediafiles']['mediafile'] : array(0 => $asset['mediafiles']['mediafile']);
    unset($asset['mediafiles']['mediafile']);
    $asset['mediafiles']['mediafile'] = $tmp;

    foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
      if ($mediafile['is_original_file'] != 'TRUE') {
        _media_mediamosa_mediafile_delete($mediafile['mediafile_id'], $mediafile['owner_id']);
      }
    }
  }

  return $mediafile_ids;
}

/**
 * Get mediafile_id and asset_id from CCK.
 *
 * @param $form
 * @param $asset_id
 * @param $mediafile_id
 *
 * @return $mediafile
 */
function _media_mediamosa_get_ids_from_cck($form, &$asset_id) {
  $asset_id = NULL;
  foreach ($form as $form_key => $form_element) {
    // CCK field.
    if (drupal_substr($form_key, 0, drupal_strlen('field_')) == 'field_') {
      foreach ($form_element as $key => $element) {
        if (is_numeric($key) && is_array($form[$form_key][$key]) && isset($form[$form_key][$key]['embed'])) {
          if (is_array($form[$form_key][$key]['embed'])) {
            $asset_id = $form[$form_key][$key]['embed']['#default_value'];
          }
          else {
            $asset_id = $form[$form_key][$key]['embed'];
          }
          break;
        }
      }
    }
  }
}

/**
 * Node form submit.
 */
function _media_mediamosa_file_upload_submit($form, &$form_state) {

  // Collect the data.
  $values = $form_state['values'];

  $title = $values['title'];
  $description = $values['body'];
  $nid = $values['nid'];
  $node_type = '_' . $values['type'];
  $asset_id = $values['asset_id'];
  // New mediafile id.
  $mediafile_id = $values['mediafile_id'];
  $mediafile_id_old = $values['mediafile_id_old'];
  $delete = $values['delete'];
  $domain_realm = $values['domain_realm'];

  if ($values['op'] == t('Create stills for the current mediafile')) {
    $still_for_old_mediafile = TRUE;
  }
  if ($values['op'] == t('Transcode the current mediafile')) {
    $transcode_profile_for_old_mediafile = TRUE;
  }

  $mediafile = _media_mediamosa_mediafile_get($mediafile_id);
  $user_id = _media_mediamosa_get_user_id();

  // Still parameters from node.
  $still_parameters = NULL;
  if (!empty($values['still_type'])) {

    // Validation.

    if ($values['size']) {
      // Get target width and height, format is 'wxh'.
      preg_match('/(\d+)x(\d+)/',  $values['size'], $matches);
      if (!is_array($matches) || count($matches) != 3 || !is_numeric($matches[1]) || !is_numeric($matches[2])) {
        drupal_set_message(t('Still size parameter has not the good format (eg. 320x240).'), 'error');
      }
    }

    if ($values['h_padding'] && !is_numeric($values['h_padding'])) {
      drupal_set_message(t('Still h padding parameter is not numeric.'), 'error');
    }

    if ($values['v_padding'] && !is_numeric($values['v_padding'])) {
      drupal_set_message(t('Still v padding parameter is not numeric.'), 'error');
    }

    if ($values['still_time_code'] && !is_numeric($values['still_time_code'])) {
      drupal_set_message(t('Still time code parameter is not numeric.'), 'error');
    }

    if ($values['still_per_mediafile'] && !is_numeric($values['still_per_mediafile'])) {
      drupal_set_message(t('Still per mediafile parameter is not numeric.'), 'error');
    }

    if ($values['still_every_second'] && !is_numeric($values['still_every_second'])) {
      drupal_set_message(t('Still every second parameter is not numeric.'), 'error');
    }

    if ($values['start_time'] && !is_numeric($values['start_time'])) {
      drupal_set_message(t('Start time of still is not numeric.'), 'error');
    }

    if ($values['end_time'] && !is_numeric($values['end_time'])) {
      drupal_set_message(t('End time of still is not numeric.'), 'error');
    }

    // Still parameters.
    $still_parameters = array(
      'still_type' => $values['still_type'],
      'size' => $values['size'],
      'h_padding' => $values['h_padding'],
      'v_padding' => $values['v_padding'],
      // Time code is not valid in the REST call.
      //'still_time_code' => $values['still_time_code'],
      'still_per_mediafile' => $values['still_per_mediafile'],
      'still_every_second' => $values['still_every_second'],
      'start_time' => $values['start_time'],
      'end_time' => $values['end_time'],
    );

    // Helper code for creating only one still.
    if (!$values['still_type'] || $values['still_type'] == MEDIA_MEDIAMOSA_STILL_TYPE_NONE) {
      if ($values['still_time_code']) {
        $still_parameters['still_type'] = MEDIA_MEDIAMOSA_STILL_TYPE_NORMAL;
        $still_parameters['still_per_mediafile'] = 1;
        $still_parameters['start_time'] = $values['still_time_code'];
        $still_parameters['end_time'] = $values['still_time_code'];
      }
    }
  }

  // Transcode parameters from node.
  $profile_id = NULL;
  if (!empty($values['transcode_profile_default'])) {
    $profile_id = $values['transcode_profile_default'];
  }

  // Get the old asset_id.
  _media_mediamosa_get_ids_from_cck($form_state['values'], $asset_id_old);


  // Save the asset_id to the node.
  foreach ($form_state['values'] as $form_key => $form_element) {
    // CCK field.
    if (drupal_substr($form_key, 0, drupal_strlen('field_')) == 'field_') {
      foreach ($form_element as $key => $element) {
        if (isset($form_state['values'][$form_key][$key]['embed'])) {
          $form_state['values'][$form_key][$key]['embed'] = $asset_id;
        }
      }
    }
  }

  if ($domain_realm) {
    preg_match_all("/[^\s]+/", $domain_realm, $match);
    $access_entries = $match[0];
  }
  else {
    $access_entries = array();
  }


  //
  // Was there file upload?
  //

  if (!((string) $mediafile['metadata']['file_duration'])) {

    //
    // There wasn't any file upload.
    //

    // Delete the mediafile, that the node form generated.
    $asset_new = _media_mediamosa_responsexml2array(_media_mediamosa_asset_get($asset_id_old));
    if (is_array($asset_new) && $asset_new['mediafiles']['mediafile']) {
      foreach ($asset_new['mediafiles']['mediafile'] as $mediafile_new) {
        if ($mediafile_new['mediafile_id'] != $mediafile_id_old && $mediafile_new['mediafile_id_source'] != $mediafile_id_old) {
          _media_mediamosa_mediafile_delete($mediafile_new['mediafile_id'], $mediafile_new['owner_id']);
        }
      }
    }


    if (!$asset_id_old) {
      // New node / asset was created without file upload.
      $asset_id_old = $asset_id;
    }

    // Security check. There should be always $asset_id_old.
    if ($asset_id_old) {
      // Create metadata for the asset.
      if (_media_mediamosa_metadata_create($asset_id_old, $user_id, $title, $description) === FALSE) {
        watchdog('media_mediamosa', 'There was a problem for creating metadata (asset_id = @asset_id, user_id = $user_id, title = @title, description = @description).', array(
          '@asset_id' => $asset_id_old,
          '@user_id' => $user_id,
          '@title' => $title,
          '@description' => $description,
        ), WATCHDOG_ERROR);
        //drupal_set_message('There was a problem for creating metadata', 'error');
        //return FALSE;
      }

      if ($delete) {
        // Delete all files, if the user checked this.
        $asset_old = _media_mediamosa_responsexml2array(_media_mediamosa_asset_get($asset_id_old));
        if ($asset_old['mediafiles']['mediafile']) {
          foreach ($asset_old['mediafiles']['mediafile'] as $mediafile_old) {
            _media_mediamosa_mediafile_delete($mediafile_old['mediafile_id'], $mediafile_old['owner_id']);
          }
        }

        if ($access_entries) {
          drupal_set_message('You can set ACL rules only if you uploaded a video.');
        }
      }
      elseif ($mediafile_id_old) {
        // Set default still.
        if (isset($values['still_images'])) {
          _media_mediamosa_still_default($asset_id_old, $mediafile_id_old, $values['still_images'], $user_id);
        }

        // Delete checked stills.
        if (!empty($form['still_images']['#options'])) {
          foreach ($form['still_images']['#options'] as $radio_key => $radio_value) {
            if (!empty($form['still_delete_' . $radio_key]) && !empty($values['still_delete_' . $radio_key]) && $values['still_delete_' . $radio_key]) {
              _media_mediamosa_still_delete($asset_id_old, $mediafile_id_old, $radio_key, $user_id);
            }
          }
        }

        // Create new stills for the old mediafile.
        if ($still_for_old_mediafile) {
          if (variable_get('media_mediamosa_still', 0) && $still_parameters) {
            _media_mediamosa_still_create($mediafile_id_old, $user_id, $still_parameters);
          }
        }

        // Transcode the old mediafile.
        if ($transcode_profile_for_old_mediafile) {
          if (variable_get('media_mediamosa_transcode_profile', 0) && $profile_id && $profile_id != 'none') {
            // Delete the old not original (transcoded) mediafiles.
            _media_mediamosa_delete_mediafile_transcoded($asset_id_old);
            // Set the transcode.
            _media_mediamosa_transcode_create($mediafile_id_old, $user_id, $profile_id);
          }
        }

        // Apply access restrictions.
        if (variable_get('media_mediamosa_access_control', 0)) {
          // Get the asset with all the mediafiles.
          $asset_get = _media_mediamosa_responsexml2array(_media_mediamosa_asset_get($asset_id_old));
          // Set the access rules to all the mediafiles.
          if (!empty($asset_get['mediafiles']['mediafile'][0])) {
            foreach ($asset_get['mediafiles']['mediafile'] as $mediafile_get) {
              _media_mediamosa_set_access_control($mediafile_get['mediafile_id'], $user_id, $access_entries);
            }
          }
        }
      }
      elseif ($access_entries) {
        drupal_set_message('You can set ACL rules only if you uploaded a video.');
      }
    }

    // Return to avoid to run the remain part of this function.
    $form_state['redirect'] = 'node/' . $nid;
    return TRUE;
  }

  //
  // There was file upload.
  //

  // The user uploaded a new mediafile, so delete old mediafiles.
  if ($asset_id_old && $mediafile_id_old) {
    $asset_old = _media_mediamosa_responsexml2array(_media_mediamosa_asset_get($asset_id_old));
    if ($asset_old['mediafiles']['mediafile']) {
      foreach ($asset_old['mediafiles']['mediafile'] as $mediafile_old) {
        if ($mediafile_old['mediafile_id'] != $mediafile_id) {
          _media_mediamosa_mediafile_delete($mediafile_old['mediafile_id'], $mediafile_old['owner_id']);
        }
      }
    }
  }

  // Create metadata for the asset.
  if (_media_mediamosa_metadata_create($asset_id, $user_id, $title, $description) === FALSE) {
    watchdog('media_mediamosa', 'There was a problem for creating metadata (asset_id = @asset_id, user_id = $user_id, title = @title, description = @description).', array(
      '@asset_id' => $asset_id,
      '@user_id' => $user_id,
      '@title' => $title,
      '@description' => $description,
    ), WATCHDOG_ERROR);
    //drupal_set_message('There was a problem for creating metadata', 'error');
    //return FALSE;
  }

  // Create still image.
  if (variable_get('media_mediamosa_still', 0) && $still_parameters) {
    _media_mediamosa_still_create($mediafile_id, $user_id, $still_parameters);
  }

  // Create transcoded mediafile.
  if (variable_get('media_mediamosa_transcode_profile', 0) && $profile_id && $profile_id != 'none') {
    // Set the transcode.
    _media_mediamosa_transcode_create($mediafile_id, $user_id, $profile_id);
  }

  // Apply access restrictions.
  if (variable_get('media_mediamosa_access_control', 0)) {
    if ($access_entries) {
      // Update to transcode_inherits_acl = TRUE;
      _media_mediamosa_mediafile_update($mediafile_id, $user_id);
      // Set the access.
      _media_mediamosa_set_access_control($mediafile_id, $user_id, $access_entries);
    }
  }

  $form_state['redirect'] = 'node/' . $nid;
}

/**
 * CCK form submit for still.
 */
function _media_mediamosa_still_settings_submit($form, &$form_state) {
  $values = $form_state['values'];
  $node_type = '_' . $values['type_name'];

  variable_set('media_mediamosa_still_local' . $node_type, $values['media_mediamosa_still_local' . $node_type]);
  variable_set('media_mediamosa_still_type' . $node_type, $values['media_mediamosa_still_type' . $node_type]);
  variable_set('media_mediamosa_still_size' . $node_type, $values['media_mediamosa_still_size' . $node_type]);
  variable_set('media_mediamosa_still_h_padding' . $node_type, $values['media_mediamosa_still_h_padding' . $node_type]);
  variable_set('media_mediamosa_still_v_padding' . $node_type, $values['media_mediamosa_still_v_padding' . $node_type]);
  variable_set('media_mediamosa_still_time_code' . $node_type, $values['media_mediamosa_still_time_code' . $node_type]);
  variable_set('media_mediamosa_still_per_mediafile' . $node_type, $values['media_mediamosa_still_per_mediafile' . $node_type]);
  variable_set('media_mediamosa_still_every_second' . $node_type, $values['media_mediamosa_still_every_second' . $node_type]);
  variable_set('media_mediamosa_still_start_time' . $node_type, $values['media_mediamosa_still_start_time' . $node_type]);
  variable_set('media_mediamosa_still_end_time' . $node_type, $values['media_mediamosa_still_end_time' . $node_type]);
}

/**
 * CCK form submit for transcode profile.
 */
function _media_mediamosa_transcode_profile_settings_submit($form, &$form_state) {
  $values = $form_state['values'];
  $node_type = '_' . $values['type_name'];

  variable_set('media_mediamosa_transcode_profile_local' . $node_type, $values['media_mediamosa_transcode_profile_local' . $node_type]);
  variable_set('media_mediamosa_transcode_profile_default' . $node_type, $values['media_mediamosa_transcode_profile_default' . $node_type]);
  if (!$values['type_name']) {
    variable_set('media_mediamosa_transcode_profile_default_options', $values['media_mediamosa_transcode_profile_default_options']);

    $profiles = _media_mediamosa_transcode_profiles_get();
    $options = array(
      'none' => t('---'),
    );
    if (is_array($profiles)) {
      foreach ($profiles as $profile_id => $profile) {
        if (in_array($profile_id, $values['media_mediamosa_transcode_profile_default_options'])) {
          $options[$profile_id] = $profile['profile'];
        }
      }
    }
    variable_set('media_mediamosa_transcode_profile_default_options_value', $options);
  }
}

/**
 * CCK form submit for access rules.
 */
function _media_mediamosa_access_settings_submit($form, &$form_state) {
  $node_type = '_' . $form_state['values']['type_name'];
  $value = $form_state['values']['domain_realm'];

  preg_match_all("/[^\s]+/", $value, $match);
  $rules = $match[0];
  variable_set('media_mediamosa_default_domains_and_realms_access' . $node_type, $rules);
}

/**
 * Get the progress of the job.
 * Menu callback for jQuery.
 */
function _media_mediamosa_get_job_progress() {
  $mediafile_id = check_plain(arg(1));
  $node_type = check_plain(arg(2));
  $node_field = unserialize(db_result(db_query("SELECT widget_settings FROM {content_node_field_instance} WHERE type_name='%s'", $node_type)));
  $autoplay = $node_field['video_autoplay'];
  $width = $node_field['video_width'];
  $height = $node_field['video_height'];
  $javascript = TRUE;

  echo _media_mediamosa_generate_video_output($mediafile_id, $node_type, $autoplay, $width, $height, $javascript);
  die;
}

/**
 * Generate video output.
 *
 * @param $mediafile_id
 * @param $node_type
 * @param $autoplay
 * @param $width
 * @param $height
 *
 * @return $output
 */
function _media_mediamosa_generate_video_output($mediafile_id, $node_type, $autoplay, $width, $height, $javascript = FALSE) {
  global $user;

  $output = '';

  $mediafile = _media_mediamosa_mediafile_get($mediafile_id);
  $asset_id = (string) $mediafile['asset_id'];
  $user_id = (string) $mediafile['owner_id'];

  $jobs = _media_mediamosa_job_get($mediafile_id, $user_id);

  $job_still = NULL;
  $job_transcode = NULL;
  if (!empty($jobs) && is_array($jobs)) {
    foreach ($jobs as $id => $job) {
      if ($job[MEDIA_MEDIAMOSA_JOB_TYPE] == MEDIA_MEDIAMOSA_JOB_TYPE_STILL) {
        // Get the last still job.
        $job_still = $job;
      }
      elseif ($job[MEDIA_MEDIAMOSA_JOB_TYPE] == MEDIA_MEDIAMOSA_JOB_TYPE_TRANSCODE) {
        // Get the last transcode job.
        $job_transcode = $job;
      }
    }
  }

  if (!$javascript && $user_id == $user->name) {
    if (!empty($job_still[MEDIA_MEDIAMOSA_JOB_STATUS]) && $job_still[MEDIA_MEDIAMOSA_JOB_STATUS] != MEDIA_MEDIAMOSA_JOB_STATUS_FINISHED && (int) $job_still['progress'] < 1) {
      if ($job_still[MEDIA_MEDIAMOSA_JOB_STATUS] != MEDIA_MEDIAMOSA_JOB_STATUS_FAILED) {
        drupal_set_message(t('Still job is working. Please, wait!'));
      }
    }
  }

  if (!empty($job_transcode[MEDIA_MEDIAMOSA_JOB_STATUS]) && $job_transcode[MEDIA_MEDIAMOSA_JOB_STATUS] != MEDIA_MEDIAMOSA_JOB_STATUS_FINISHED && (int) $job_transcode['progress'] < 1) {
    if ($job_transcode[MEDIA_MEDIAMOSA_JOB_STATUS] == MEDIA_MEDIAMOSA_JOB_STATUS_FAILED) {
      $output .= t('Transcode error.');
    }
    else {
      if ($user_id == $user->name) {
        $output .= theme('job_progress_bar', $mediafile_id, $node_type, 100 * $job_transcode['progress'],
          $job_transcode[MEDIA_MEDIAMOSA_JOB_STATUS] == MEDIA_MEDIAMOSA_JOB_STATUS_WAITING ? t('The mediafile transcode starts soon. Please, wait.') : t('The mediafile is under transcoding. Please, wait.')
        );
      }
      else {
        $output .= t('Video transcode in progress, please wait.');
      }
    }
  }
  else {
    // Get the asset.
    $asset = _media_mediamosa_asset_get($asset_id);

    // Search the transcoded mediafile (if there is any).
    if (!empty($asset['mediafiles']['mediafile'])) {
      foreach ($asset['mediafiles']['mediafile'] as $key => $mediafile) {
        if ($mediafile['transcode_profile_id']) {
          $mediafile_id = (string) $mediafile['mediafile_id'];
          break;
        }
      }
    }

    if (!$mediafile_id) {
      return FALSE;
    }

    // Get the video object.
    $response = _media_mediamosa_connector('asset/' . $asset_id . '/play?mediafile_id=' . $mediafile_id . '&user_id=' . _media_mediamosa_get_user_id() . '&autostart=' . ($autoplay ? 'TRUE' : 'FALSE') . '&width=' . $width . '&height=' . $height . '&response=object');
    if (!$response) {
      return FALSE;
    }

    if ($response->xml->items->item->output) {
      $output .= (string) $response->xml->items->item->output;
    } elseif ($response->xml->header->request_result_id == ERRORCODE_STREAMING_PROFILE_NOT_FOUND) {
      // We wait, if the transcoded mediafile still not ready.
      if ($user_id == $user->name) {
        $output .= theme('job_progress_bar', $mediafile_id, $node_type, 100 * $job_transcode['progress'], t('The mediafile is under transcoding. Please, wait.'));
      }
      else {
        $output .= t('Video transcode in progress, please wait.');
      }
    }
    elseif ($response->xml->header->request_result_id == ERRORCODE_NOT_AUTHORIZED) {
      $output .= t('Not authorized to view video!');
      watchdog('media_mediamosa', 'Not authorized to view video. Response = @response', array('@response' => print_r($response, TRUE)), WATCHDOG_ERROR);
    }
    else {
      $output .= t('It is not media file or not playable!');
      watchdog('media_mediamosa', 'It is not media file or not playable. Response = @response', array('@response' => print_r($response, TRUE)), WATCHDOG_ERROR);
    }
  }

  return $output;
}

function theme_job_progress_bar($mediafile_id, $node_type, $percent, $message) {
  $output = '';
  // Container.
  $output .= '<div id="progress_' . $mediafile_id . '" class="progress" style="width: ' . MEDIA_MEDIAMOSA_PROGRESS_BAR_SIZE . 'px">';
  // jQuery script.
  $output .= '<script src="/misc/jquery.js"></script>';
  // Normal progress elements.
  $output .= '  <div class="bar"><div class="filled" style="width: ' . $percent . '%"></div></div>';
  $output .= '  <div class="percentage">' . $percent . '%</div>';
  $output .= '  <div class="message">' . $message . '</div>';
  // Our script (it reloads the container after a time).
  $output .= '<script type="text/javascript">
<!--//--><![CDATA[//><!--

setTimeout("mediamosa_job_progressbar()", ' . MEDIA_MEDIAMOSA_PROGRESS_BAR_TIME . ');

function mediamosa_job_progressbar() {
  try {
    $("#progress_' . $mediafile_id . '").load("/get_job_progress/' . $mediafile_id . '/' . $node_type . '");
  }
  catch (err) {
    window.location.reload();
  }
}

//--><!]]>
</script>';
  // Close container.
  $output .= '</div>';

  return $output;
}

/**
 * Check if input is realm.
 */
function _media_mediamosa_is_realm($domain_realm) {
  return ((strpos($domain_realm, '@') === FALSE) ? FALSE : TRUE);
}

/**
 * Check if input is domain.
 */
function _media_mediamosa_is_domain($domain_realm) {
  // If its a realm, its not a domain and visa versa.
  return !_media_mediamosa_is_realm($domain_realm) && !empty($domain_realm);
}

/**
 * Get running jobs.
 * And create a form item for that information.
 *
 * @param &$form
 * @param $mediafile_id
 * @param $job_type
 */
function _media_mediamosa_get_running_jobs(&$form, $mediafile_id, $job_type) {
  $jobs = _media_mediamosa_job_get($mediafile_id, _media_mediamosa_get_user_id());
  $jobs_count = 0;
  if ($jobs) {
    foreach ($jobs as $job) {
      if ($job[MEDIA_MEDIAMOSA_JOB_STATUS] != MEDIA_MEDIAMOSA_JOB_STATUS_FINISHED && $job[MEDIA_MEDIAMOSA_JOB_STATUS] != MEDIA_MEDIAMOSA_JOB_STATUS_FAILED && $job[MEDIA_MEDIAMOSA_JOB_TYPE] == $job_type) {
        $jobs_count++;
      }
    }
  }
  if ($jobs_count) {
    $form['running_jobs'] = array(
      '#type' => 'item',
      '#title' => t('Running @job_type jobs', array('@job_type' => drupal_strtolower($job_type),)),
      '#value' => t('There are @num @job_type job(s) running.', array('@job_type' => drupal_strtolower($job_type), '@num' => $jobs_count)),
    );
  }
}

/**
 * Parse file duration.
 *
 * @param string $file_duration
 *
 * @return $duration
 */
function _media_mediamosa_parse_duration($file_duration) {
  // Parse the duration. Eg. "00:00:30.14"
  $file_duration = explode(':', $file_duration);
  $duration = 0;
  if (!empty($file_duration)) {
    foreach ($file_duration as $part) {
      $duration = 60 * $duration + $part;
    }
  }

  return $duration;
}

/**
 * Set MediaMosa CCK fields.
 *
 * @param object $node
 * @param string $asset_id
 * @param int $duration
 *
 * @return $node
 */
function _media_mediamosa_set_mediamosa_cck_fields($node, $asset_id, $duration) {
  foreach ($node as $node_key => $node_element) {
    // CCK field.
    if (drupal_substr($node_key, 0, drupal_strlen('field_')) == 'field_') {
      foreach ($node_element as $key => $element) {
        if (is_numeric($key) && is_array($node->{$node_key}[$key]) && array_key_exists('embed', $node->{$node_key}[$key])) {
          if (is_array($node->{$node_key}[$key]['embed'])) {
            $node->{$node_key}[$key]['embed']['#default_value'] = $asset_id;
            $node->{$node_key}[$key]['value']['#default_value'] = $asset_id;
          }
          else {
            $node->{$node_key}[$key]['embed'] = $asset_id;
            $node->{$node_key}[$key]['value'] = $asset_id;
          }
          break;
        }
      }
    }
  }

  return $node;
}


/**
 * Check a node object for presence of our embedded video field.
 *
 * @param object $node
 *
 * @return boolean
 */
function _media_mediamosa_have_mediafield($node) {
  $type = db_result(db_query("SELECT type_name FROM {content_node_field_instance} WHERE widget_module = 'emvideo' AND widget_settings LIKE '%%mediamosa%%' AND type_name = '%s'", $node->type));
  return (boolean)$type;
}

/**
 * Helper function for debugging.
 * Usage: d() or d($some_variable) or d($some_variable, 'variable_name');.
 * Third parameter can be the visible width of the result.
 */
if (!function_exists('d')) {
  function d($variable = 'I was here', $name = 'variable', $width = NULL) {
    drupal_set_message($name . ': <pre' . ($width ? ' width=' . $width : '') . '>' . htmlspecialchars(print_r($variable, TRUE)) . '</pre>');
  }
}

/**
 * Helper function for debugging, no Drupal required.
 * Usage: dr() or dr($some_variable);.
 */
if (!function_exists('dr')) {
  function dr($variable = 'I was here', $name = 'variable', $width = NULL) {
    $out = htmlspecialchars(print_r($variable, TRUE));
    if ($width) {
      $output = '';
      $space = 0;
      for ($i = 0; $i < drupal_strlen($out); $i++) {
        $output .= $out[$i];
        if ($out[$i] != ' ') {
          $space++;
        }
        else {
          $space = 0;
        }
        if ($space >= $width) {
          $space = 0;
          $output .= '<br />';
        }
      }
    }
    else {
      $output = $out;
    }
    echo $name . ': <pre>' . $output . '</pre><br />';
  }
}
