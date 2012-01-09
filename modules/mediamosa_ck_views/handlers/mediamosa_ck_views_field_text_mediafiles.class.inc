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

class mediamosa_ck_views_field_text_mediafiles extends views_handler_field_xss {

  function option_definition() {
    $options = parent::option_definition();

    $options['mediafile_theme'] = array('default' => 'mediamosa_ck_view_theme_asset_mediafiles');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['mediafile_theme'] = array(
      '#title' => t('Theme function to use'),
      '#type' => 'textfield',
      '#default_value' => $this->options['mediafile_theme'],
    );
  }

  /**
   * Get the value that's supposed to be rendered.
   *
   * @param $values
   *   An object containing all retrieved values.
   * @param $field
   *   Optional name of the field where the value is stored.
   */
  function get_value($values, $field = NULL) {

    // Get the data.
    $mediafiles = parent::get_value($values, $field);

    return theme($this->options['mediafile_theme'], array('mediafiles' => $mediafiles));
  }


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
}
