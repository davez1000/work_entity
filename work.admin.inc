<?php

/**
 * Menu callback; List all work types available.
 */
function work_overview_types() {
  foreach (work_types() as $type => $info) {
    $type_url_str = str_replace('_', '-', $type);
    $label = t('View @type', array('@type' => $info->name));
    $items[] = l($label, 'admin/structure/works/manage/' . $type_url_str);
  }

  return theme('item_list', array('items' => $items));
}

/**
 * Menu callback; work information page.
 *
 * @param object $work_type
 */
function work_information($work_type) {
  return $work_type->name . ': ' . $work_type->description;
}
