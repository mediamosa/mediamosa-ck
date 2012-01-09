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

class mediamosa_ck_views_filter_text extends views_handler_filter {

  public function query() {
    if (!empty($this->value)) {
      $value = $this->value;
      $fieldname = $this->field;
      if (is_array($value)) {
        $values = array_filter($value);
        // Ensure that some values have been selected.
        if (!empty($values)) {
          $this->query->add_filter($fieldname, '(' . implode('OR', $values) . ')');
        }
        return;
      }
      $this->query->add_filter($fieldname, $value);
    }
  }

  /**
   * Provide a simple textfield for equality
   */
  function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => t($this->definition['title']),
      '#default_value' => $this->value,
    );
  }
}
