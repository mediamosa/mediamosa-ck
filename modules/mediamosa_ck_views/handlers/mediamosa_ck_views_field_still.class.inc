<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl)
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

class mediamosa_ck_views_field_still extends views_handler_field {
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

    $options['still_style'] = array('default' => TRUE);

    return $options;
  }

  /**
   * Still options.
   */
  function options_form(&$form, &$form_state) {

    // File the options.
    $styles = array();
    foreach (image_styles() as $style_name => $image_style) {
      $styles[$style_name] = $style_name;
    }

    $form['still_style'] = array(
      '#title' => t('Select image style'),
      '#type' => 'select',
      '#options' => $styles,
      '#default_value' => $this->options['still_style'],
    );
    parent::options_form($form, $form_state);
  }

  function render($values) {
    $variables = array(
      'style_name' => $this->options['still_style'],
      // @todo: should be an real path instead of public://.
      'path' => MediaMosaStreamWrapper::mediamosa_getLocalThumbnailPath($values->asset_id),
    );

    return theme('image_style', $variables);
  }
}
