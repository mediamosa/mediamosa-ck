<?php
/**
 * @file
 * Base class for filter classes.
 */

class mediamosa_ck_views_filter extends views_handler_filter {

  /**
   * In case you need to alter one value in another type.
   *
   * @param string $value
   *   The value to change.
   *
   * @return string
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
