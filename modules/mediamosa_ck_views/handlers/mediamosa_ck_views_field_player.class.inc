<?php
/**
 * @file
 * Views field player.
 */

class mediamosa_ck_views_field_player extends views_handler_field {
  // The following methods express strong SQLisms in the base field handler
  // class. Override and blank them out.
  public function pre_render(&$values) {
  }

  public function add_additional_fields($fields = NULL) {
    foreach ($fields as $field) {
      $this->aliases[$field] = $this->query->add_field($this->table, $field);
    }
  }

  public function click_sort($order) {
    $this->query->add_orderby($this->table, $this->real_field, $order);
  }

  public function query() {
    $this->field_alias = $this->query->add_field($this->table, $this->real_field);

    // Add in additional fields.
    if (!empty($this->additional_fields)) {
      $this->add_additional_fields($this->additional_fields);
    }
  }

  /**
   * Setup default for options.
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['response'] = array('default' => 'object');
    $options['player_width'] = array('default' => '640');
    $options['player_height'] = array('default' => '360');
    $options['player_autostart'] = array('default' => 0);

    return $options;
  }

  /**
   * Still options.
   */
  public function options_form(&$form, &$form_state) {

    $options = array(
      'plain' => 'Play URL',
      'object' => 'HTML object code',
    );

    $form['response'] = array(
      '#title' => t('Select response type'),
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $this->options['response'],
    );
    $form['player_width'] = array(
      '#title' => t('Player width'),
      '#description' => t('Only for response HTML object code.'),
      '#type' => 'textfield',
      '#default_value' => $this->options['player_width'],
    );
    $form['player_height'] = array(
      '#title' => t('Player height'),
      '#description' => t('Only for response HTML object code.'),
      '#type' => 'textfield',
      '#default_value' => $this->options['player_height'],
    );
    $form['player_autostart'] = array(
      '#title' => t('Autostart player'),
      '#description' => t('Only for response HTML object code. Does not work on all players.'),
      '#type' => 'select',
      '#options' => array(0 => 'No', 1 => 'Yes'),
      '#default_value' => $this->options['player_autostart'],
    );
    parent::options_form($form, $form_state);
  }

  public function render($values) {
    // To render, asset_id must be present.
    if (empty($values->asset_id)) {
      return;
    }

    // Empty at start.
    $options = array();

    // Possible start/end time from _GET['t'] parameter.
    if (isset($_GET['t'])) {

      $matches = array();
      preg_match('/(?P<start_sec>([0-9]*\.?[0-9]*))(?P<comma>\,?)(?P<end_sec>([0-9]*\.?[0-9]*))/', check_plain($_GET['t']), $matches);
      $options['start'] = 0;
      if (isset($matches['start_sec'])) {
        $options['start'] = $matches['start_sec'] * 1000;
      }
      if (isset($matches['end_sec'])) {
        $options['duration'] = ($matches['end_sec'] * 1000) - $options['start'];
        if ($options['duration'] < 0) {
          unset($options['duration']);
        }
      }
    }

    $this->options['player_autostart'] = isset($_GET['autoplay']) && (drupal_strtolower($_GET['autoplay']) === 'true' || $_GET['autoplay'] === '1');

    // What mediafile to play?
    $mediafile_id_to_play = empty($values->mediafile_id_to_play) ? NULL : $values->mediafile_id_to_play;

    // Take user_id of current watcher.
    $user_id = mediamosa_ck::session_user_id();

    // Determine admin user.
    if (user_access('access administration pages')) {
      $options['is_app_admin'] = TRUE;
    }

    // Response.
    $options['response'] = $this->options['response'];

    // Both not empty.
    if (!empty($this->options['player_width']) && !empty($this->options['player_height'])) {
      $options['width'] = $this->options['player_width'];
      $options['height'] = $this->options['player_height'];
    }

    $options['autostart'] = $this->options['player_autostart'] ? 'TRUE' : 'FALSE';

    try {
      $options['fatal'] = TRUE;
      $response = MediaMosaCkConnectorWrapper::get_play_link($values->asset_id, $mediafile_id_to_play, $user_id, $options);
      $response = theme('mediamosa_ck_views_player', array('response' => $response));
    }
    catch (Exception $e) {
      $response = theme('mediamosa_ck_views_player_image',
        array(
          'width' => $options['width'],
          'height' => $options['height'],
          'error_code' => $e->getCode(),
          'asset_id' => $values->asset_id,
          'mediafile_id' => $mediafile_id_to_play,
        ));
    }
    return $response;
  }
}
