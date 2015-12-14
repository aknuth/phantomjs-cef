    function examineTree(jElm){
        jElm.children().each(function(){ 
            if ( $(this).is($('div.table_quotes'))){
                return;
            } else if ( $(this).find('div.table_quotes').length === 0  ) {
                console.log('--- '+$(this).prop("tagName"));
                $(this).remove();
            } else {
                console.log('+++ '+$(this).prop("tagName"));
                examineTree($(this));
            }
        })
    }
