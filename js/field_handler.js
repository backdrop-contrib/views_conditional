(function($) {
  Drupal.behaviors.fieldHandler = {
    condition_count: 1,

    attach: function() {
      $('#field-add-and').click(function() {
        Drupal.behaviors.fieldHandler.add_element('and');
        return false;
      });
      $('#add-condition').click(function() {
        Drupal.behaviors.fieldHandler.add_condition();
        return false;
      });
    },

    add_element: function(type) {
      var item = $('<div></div>').addClass('list-item');
      var c = $('.form-item-options-if select.first-field').clone();
      c.removeClass('first-field').attr('name', 'options[if][more][' + type + '][]').appendTo(item);
      c.before(Drupal.t(type) + ' ');

      var cross = $('<a></a>');
      cross.text('X');
      cross.attr('href', '#');
      cross.click(function() {
        $(this).parent().remove();
      });
      c.after(cross);
      c.after('&nbsp;');

      item.appendTo($('.form-item-options-if'));
    },

    add_condition: function(type) {
      console.log("OH MY GOODNESS");
      var item = $('<div></div>').addClass('new-condition');
      var c = $('#conditions-container').clone();
      c.find('#edit-options-conditions-more').remove();
      c.find('.condition-number').text('#' + ++Drupal.behaviors.fieldHandler.condition_count);
      c.find('select').attr('name', 'options[conditions][more][condition][]').val("");
      c.find('input').attr('name', 'options[conditions][more][equalto][]').val("");
      c.appendTo(item);

      var cross = $('<a></a>');
      cross.text('X');
      cross.attr('href', '#');
      cross.click(function() {
        $(this).parent().parent().parent().remove();
        Drupal.behaviors.fieldHandler.condition_count--;
        Drupal.behaviors.fieldHandler.redo_numbers();
      });
      c.find('.form-type-select label').append(cross);

      item.appendTo($('#new-conditions-container'));
    },

    redo_numbers: function() {
      var i = 1;
      $('#new-conditions-container .condition-number').each(function() {
        i++;
        $(this).text('#' + i);
      });
    }
  };
})(jQuery);