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
 * Base class for filter classes.
 */

class mediamosa_ck_views_filter extends views_handler_filter {

  /**
   * In case you need to alter one value in another type.
   *
   * @param $value
   *   The value to change.
   *
   * @return
   *   Return the converted value.
   */
  protected function _convert_value($value) {
    return $value;
  }

  /**
   * Adds filter to CQL for exact matching.
   */
  public function _cql_exact() {
    if (!empty($this->value)) {
      $cql_or = array();
      $values = array_filter(is_array($this->value) ? $this->value : array($this->value));

      foreach ($values as $value) {
        if (is_array($value)) {
          foreach ($value as $value_child) {
            $cql_or[] = $this->field . '=="^' . $this->_convert_value($value_child) . '^"';
          }
        }
        else {
          $cql_or[] = $this->field . '=="^' . $this->_convert_value($value) . '^"';
        }
      }

      // Add to filter.
      $this->query->add_filter($this->field, count($cql_or) > 1 ? '(' . implode(' OR ', $cql_or) . ')' : reset($cql_or));
    }
  }

  /**
   * Adds filter to CQL for loose matching.
   */
  public function _cql() {
    if (!empty($this->value)) {
      $cql_or = array();
      $values = array_filter(is_array($this->value) ? $this->value : array($this->value));

      foreach ($values as $value) {
        if (is_array($value)) {
          foreach ($value as $value_child) {
            $cql_or[] = $this->field . '="' . $value_child . '"';
          }
        }
        else {
          $cql_or[] = $this->field . '="' . $value . '"';
        }
      }

      $this->query->add_filter($this->field, count($cql_or) > 1 ? '(' . implode(' OR ', $cql_or) . ')' : reset($cql_or));
    }
  }
}
