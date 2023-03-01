	struct {
		const struct attr_info* table;
		const size_t length;
		tcflag_t* flag_set;
	} lookup_tables[] = {
		{ arrandsize(LOOKUP_IFLAGS), &(term->c_iflag) },
		{ arrandsize(LOOKUP_OFLAGS), &(term->c_oflag) },
		{ arrandsize(LOOKUP_LFLAGS), &(term->c_lflag) },
	};
	
	const struct attr_info* attr = NULL;
	tcflag_t* flag_set = NULL;
	
	for (size_t i = 0; i < sizeofarr(lookup_tables); i++) {
		if ((attr = find_attr_info(
			name,
			lookup_tables[i].table,
			lookup_tables[i].length
		)) != NULL) {
			flag_set = lookup_tables[i].flag_set;
			break;
		}
	}
	
	if (flag_set == NULL) return false;
