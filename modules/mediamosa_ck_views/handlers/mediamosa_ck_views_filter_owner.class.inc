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

/**
 * @file
 * Filter for collection selection in an exposed form.
 */

class mediamosa_ck_views_filter_owner extends mediamosa_ck_views_filter {

  /**
   * We need to convert from Drupal name to Drupal mail address.
   *
   * @param $value
   *   The value to change.
   *
   * @return
   *   Return the converted value.
   */
  public function _convert_value($value) {

    // Get the user.
    $user = mediamosa_ck::lookup_user_with_name($value);

    // If user can not be found, then we return the original.
    return empty($user) ? $value : $user->mail;
  }

  /**
   * Add input to filter data.
   */
  public function query() {
    parent::_cql_exact();
  }

  /**
   * Provide a simple textfield for equality
   */
  public function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => t($this->definition['title']),
      '#default_value' => $this->value,
      '#autocomplete_path' => 'user/autocomplete',
    );
  }
}
