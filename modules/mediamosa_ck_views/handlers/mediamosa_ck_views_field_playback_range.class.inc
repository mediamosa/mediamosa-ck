<?php
/**
 * @file
 * Views field for playback range.
 */

class mediamosa_ck_views_field_playback_range extends views_handler_field {

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
    // Nothing to render.
  }
}
