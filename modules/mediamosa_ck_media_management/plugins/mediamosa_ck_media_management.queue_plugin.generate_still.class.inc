<?php
/**
 * @file
 * Generate still plugin.
 */

class mediamosa_ck_media_management_queue_plugin_generate_still extends mediamosa_ck_media_management_queue_plugin {
  // ------------------------------------------------------------------- Consts.
  const STILL_TYPE_NONE = 'NONE';
  const STILL_TYPE_SECOND = 'SECOND';
  const STILL_TYPE_NORMAL = 'NORMAL';
  const STILL_TYPE_SCENE = 'SCENE';

  // Still per mediafile
  const STILL_PER_MEDIAFILE = 3;
  // Still on every second
  const STILL_EVERY_SECOND = 5;
  // Start time
  const STILL_START_TIME = 0;
  // End frame
  const STILL_END_TIME = 9999999;
  // Size
  const STILL_SIZE = '352x288';
  // H padding
  const STILL_H_PADDING = 0;
  // V padding
  const STILL_V_PADDING = 0;

  // ---------------------------------------------------------------- Functions.
  /**
   * Get the settings form.
   *
   * Return empty array or FALSE when the process has no settings.
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   * @param array $batch
   *   The batch data.
   *
   * @return array
   *   The settings form.
   */
  public static function settings_form($form, &$form_state, $batch) {
    return self::still_form($form, $form_state);
  }

  /**
   * Process the asset.
   *
   * @param mediamosa_connector_response_asset $response
   *   The response from the asset call.
   * @param array $data
   *   Array of data that has been stored from the last process_asset() call.
   *   The 'form_values' contains the submit of the possible options form
   *   submit.
   *
   * @return array
   *   Return the data array again to be stored for the next call.
   */
  public function process_asset(mediamosa_connector_response_asset $response, array $data = array()) {
    $user_id = $this->get_session_id();

    // Setup default still options.
    $data += array(
      'still_type' => self::STILL_TYPE_NONE,
      'still_per_mediafile' => self::STILL_PER_MEDIAFILE,
      'still_every_second' => self::STILL_EVERY_SECOND,
      'start_time' => self::STILL_START_TIME,
      'end_time' => self::STILL_END_TIME,
      'size' => self::STILL_SIZE,
      'h_padding' => self::STILL_H_PADDING,
      'v_padding' => self::STILL_V_PADDING,
    );
    $options = array(
      'still_type' => $data['still_type'],
      'still_per_mediafile' => $data['still_per_mediafile'],
      'still_every_second' => $data['still_every_second'],
      'start_time' => $data['start_time'],
      'end_time' => $data['end_time'],
      'size' => $data['size'],
      'h_padding' => $data['h_padding'],
      'v_padding' => $data['v_padding'],
    );

    // Find original mediafiles.
    $mediafiles = $response->get_original_mediafiles();

    // Loop through the mediafile IDs.
    foreach ($mediafiles as $mediafile_id => $mediafile) {
      MediaMosaCkConnectorWrapper::create_mediafile_still($mediafile_id, $user_id, $options);
    }

    return $data;
  }

  /**
   * Our still settings form.
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   *
   * @return array
   *   The still settings form.
   */
  public static function still_form($form, &$form_state) {
    // Get possible values.
    $values = empty($form_state['values']) ? array() : $form_state['values'];

    // Get default still type.
    $still_type = empty($values['still_type']) ? self::STILL_TYPE_NONE : $values['still_type'];

    $form['still']['still_type'] = array(
      '#title' => t('Select your still type'),
      '#type' => 'select',
      '#options' => array(
        self::STILL_TYPE_NONE => 'Default',
        self::STILL_TYPE_NORMAL => '# of stills per mediafile',
        self::STILL_TYPE_SECOND => 'A still every # of seconds',
        self::STILL_TYPE_SCENE => 'A still for each scene',
      ),
      '#size' => 1,
      '#required' => TRUE,
      '#default_value' => $still_type,
      '#ajax' => array(
        'callback' => '_mediamosa_ck_media_management_plugin_form_wrapper',
        'wrapper' => 'mediamosa-ck-media-management-plugin-form-wrapper',
        'method' => 'replace',
        'event' => 'change',
        'progress' => array('message' => ''),
      ),
    );

    $form['still']['size'] = array(
      '#title' => t('Size of still (in pixels, WidthxHeight)'),
      '#type' => 'textfield',
      '#size' => 15,
      '#default_value' => variable_get('mediamosa_ck_mm_still_size', self::STILL_SIZE),
      '#required' => TRUE,
    );

    $form['still']['h_padding'] = array(
      '#title' => t('Horizontal padding (in pixels)'),
      '#type' => 'textfield',
      '#size' => 4,
      '#default_value' => variable_get('mediamosa_ck_mm_still_h_padding', self::STILL_H_PADDING),
      '#required' => TRUE,
    );

    $form['still']['v_padding'] = array(
      '#title' => t('Vertical padding (in pixels)'),
      '#type' => 'textfield',
      '#size' => 4,
      '#default_value' => variable_get('mediamosa_ck_mm_still_v_padding', self::STILL_V_PADDING),
      '#required' => TRUE,
    );

    if ($still_type == self::STILL_TYPE_NORMAL) {
      $form['still']['still_per_mediafile'] = array(
        '#title' => t('# of stills per mediafile'),
        '#type' => 'textfield',
        '#size' => 3,
        '#default_value' => variable_get('mediamosa_ck_mm_still_per_mediafile', self::STILL_PER_MEDIAFILE),
      );
    }

    if ($still_type == self::STILL_TYPE_SECOND) {
      $form['still']['still_every_second'] = array(
        '#title' => t('One still every # of seconds'),
        '#type' => 'textfield',
        '#size' => 6,
        '#default_value' => variable_get('mediamosa_ck_mm_still_every_second', self::STILL_EVERY_SECOND),
      );
    }

    if ($still_type == self::STILL_TYPE_NORMAL || $still_type == self::STILL_TYPE_SECOND) {
      $form['still']['start_time'] = array(
        '#title' => t('Start time in seconds'),
        '#type' => 'textfield',
        '#size' => 7,
        '#default_value' => variable_get('mediamosa_ck_mm_still_start_time', self::STILL_START_TIME),
      );

      $form['still']['end_time'] = array(
        '#title' => t('End time in seconds'),
        '#type' => 'textfield',
        '#size' => 7,
        '#default_value' => variable_get('mediamosa_ck_mm_still_end_time', self::STILL_END_TIME),
      );
    }
    return $form;
  }
}
