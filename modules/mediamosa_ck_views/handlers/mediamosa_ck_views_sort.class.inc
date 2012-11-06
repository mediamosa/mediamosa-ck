<?php
/**
 * @file
 * Views sort.
 */

class mediamosa_ck_views_sort extends views_handler_sort {

  /**
   * Function query.
   */
  public function query() {
    $this->query->add_orderby($this->table, $this->real_field, $this->options['order']);
  }
}
