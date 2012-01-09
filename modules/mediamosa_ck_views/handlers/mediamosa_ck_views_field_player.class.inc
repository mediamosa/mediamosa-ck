<?php
/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl)
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

class mediamosa_ck_views_field_player extends views_handler_field {
  // The following methods express strong SQLisms in the base field handler
  // class. Override and blank them out.
  function pre_render() {
  }

  function add_additional_fields($fields = NULL) {
    foreach ($fields as $field) {
      $this->aliases[$field] = $this->query->add_field($this->table, $field);
    }
  }

  function click_sort($order) {
    $this->query->add_orderby($this->table, $this->real_field, $order);
  }

  function query() {
    $this->field_alias = $this->query->add_field($this->table, $this->real_field);

    // Add in additional fields.
    if (!empty($this->additional_fields)) {
      $this->add_additional_fields($this->additional_fields);
    }
  }

  /**
   * Setup default for options.
   */
  function option_definition() {
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
  function options_form(&$form, &$form_state) {

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

  function render($values) {

    if (empty($values->asset_id)) {
      return;
    }

    // What mediafile to play?
    $mediafile_id_to_play = empty($values->mediafile_id) ? NULL : $values->mediafile_id;

    // Take user_id of current watcher.
    $user_id = mediamosa_ck::session_user_id();

    // Response.
    $options['response'] = $this->options['response'];

    // Not specified? Then use default transcode profile ID.
    if (empty($mediafile_id_to_play)) {
      $options['profile_id'] = mediamosa_ck_transcode::get_default_transcode_profile();
    }

    // Both not empty.
    if (!empty($this->options['player_width']) && !empty($this->options['player_height'])) {
      $options['width'] = $this->options['player_width'];
      $options['height'] = $this->options['player_height'];
      $options['autostart'] = $this->options['player_autostart'] ? 'TRUE' : 'FALSE';
    }

    // Accept as is.
    return MediaMosaCkConnectorWrapper::get_play_link($values->asset_id, $mediafile_id_to_play, $user_id, $options);
  }
}
