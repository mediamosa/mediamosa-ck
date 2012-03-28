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

class mediamosa_ck_views_field_owner extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();

    $options['user_empty_name'] = array('default' => '-');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['user_empty_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Default user name'),
      '#description' => t(''),
      '#default_value' => isset($this->options['user_empty_name']) ? $this->options['user_empty_name'] : '',
    );

    parent::options_form($form, $form_state);
  }

  // The following methods express strong SQLisms in the base field handler
  // class. Override and blank them out.
  function pre_render(&$values) {
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

  function render($values) {
    // Must exist.
    assert(isset($values->owner_id));

    // Lookup the owner.
    $user = mediamosa_ck::lookup_user_with_mail($values->owner_id);

    if (empty($user)) {
      $user = new stdClass();
      $user->uid = 0;
      $user->name = $this->options['user_empty_name'];
    }

    return theme('mediamosa_ck_views_theme_owner', array('uid' => $user->uid, 'name' => $user->name));
  }
}
