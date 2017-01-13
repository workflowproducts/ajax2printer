-- DROP SCHEMA zbr;

CREATE SCHEMA zbr
  AUTHORIZATION postgres;

COMMENT ON SCHEMA zbr IS 'Zebra Printer functions';

GRANT USAGE,CREATE ON SCHEMA zbr TO postgres;
GRANT USAGE ON SCHEMA zbr TO public;

-- DROP FUNCTION zbr.wide_dot(int_narrow_dots integer, num_ratio numeric);

CREATE OR REPLACE FUNCTION zbr.wide_dot(int_narrow_dots integer, num_ratio numeric)
  RETURNS integer AS
$BODY$
DECLARE

BEGIN
  RETURN (int_narrow_dots::numeric * num_ratio)::integer;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.wide_dot(int_narrow_dots integer, num_ratio numeric) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.wide_dot(int_narrow_dots integer, num_ratio numeric) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.wide_dot(int_narrow_dots integer, num_ratio numeric) TO public;

--SELECT zbr.wide_dot(int_narrow_dots integer, num_ratio numeric);


-- DROP FUNCTION zbr.v_letter_width(str_font text, str_letter text);

CREATE OR REPLACE FUNCTION zbr.v_letter_width(str_font text, str_letter text)
  RETURNS integer AS
$BODY$
DECLARE
  num_width numeric;

BEGIN
  CASE upper(str_letter)
  WHEN 'I' THEN num_width := 0.25;
  WHEN 'E', 'F', 'L' THEN num_width := 0.42;
  WHEN 'P', 'T' THEN num_width := 0.478;
  WHEN 'C', 'H', 'J', 'Q' THEN num_width := 0.507;
  WHEN 'B', 'D', 'K', 'R', 'V', 'X', 'Y' THEN num_width := 0.535;
  WHEN 'A', 'G', 'N', 'O', 'U', 'S' THEN num_width := 0.563;
  WHEN 'Z' THEN num_width := 0.591;
  WHEN 'M', '-' THEN num_width := 0.7;
  WHEN 'W' THEN num_width := 0.73;
  ELSE num_width := 0.563;
  END CASE;
  RETURN (num_width * zbr.font_width(str_font)::numeric)::integer;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.v_letter_width(str_font text, str_letter text) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.v_letter_width(str_font text, str_letter text) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.v_letter_width(str_font text, str_letter text) TO public;

--SELECT zbr.v_letter_width(str_font text, str_letter text);


-- DROP FUNCTION zbr.test();

CREATE OR REPLACE FUNCTION zbr.test()
  RETURNS SETOF record AS
$BODY$
DECLARE

BEGIN
RETURN QUERY (
SELECT 1 AS sort, 'center39' AS function_name, zbr.center39('12-1234.10a', 800, 2, 2::numeric)::text
UNION
SELECT 2 AS sort, 'center39' AS function_name, zbr.center39('-PART NUMBER-', 800, 2, 2.5::numeric)::text
UNION
SELECT 3 AS sort, 'center39' AS function_name, zbr.center39('DGHJLQVWXYZ-A', 800, 2, 2.5::numeric)::text
UNION
SELECT 4 AS sort, 'center39' AS function_name, zbr.center39('L12345Q123X12', 600, 2, 2.5::numeric)::text
UNION
SELECT 5 AS sort, 'center_font' AS function_name, zbr.center_font('RA010', 'V', 800)::text
UNION
SELECT 6 AS sort, 'center_font' AS function_name, zbr.center_font('AIRCRAFT', 'V', 800)::text
UNION
SELECT 7 AS sort, 'v_letter_width' AS function_name, zbr.v_letter_width('T', 'M')::text
ORDER BY sort ASC
);
RETURN;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;

ALTER FUNCTION zbr.test() OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.test() TO postgres;
GRANT EXECUTE ON FUNCTION zbr.test() TO public;

--SELECT zbr.test();


-- DROP FUNCTION zbr.string_width(str_value text, str_font text);

CREATE OR REPLACE FUNCTION zbr.string_width(str_value text, str_font text)
  RETURNS integer AS
$BODY$
DECLARE
  str_working text := str_value;
  int_count integer := 0;
  str_letter text;

BEGIN
  WHILE length(str_working) > 0 LOOP
    str_letter := substring(str_working FROM 1 FOR 1);
    str_working := substring(str_working FROM 2);
    int_count := int_count + zbr.v_letter_width(str_font, str_letter);
  END LOOP;
  RETURN int_count;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.string_width(str_value text, str_font text) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.string_width(str_value text, str_font text) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.string_width(str_value text, str_font text) TO public;

--SELECT zbr.string_width(str_value text, str_font text);


-- DROP FUNCTION zbr.sample_report();

CREATE OR REPLACE FUNCTION zbr.sample_report()
  RETURNS text AS
$BODY$
DECLARE
  str_content text;
  int_lot_no integer := 53210;
  str_pn text := '12345678901234567890123456789012345';
  int_qty integer := 350;
  str_location text := '1234';
  str_po_no text := '123456';
  dat_po_received date := '12/12/2012'::date;
  str_mfrname text := '12345678901234567890123456789012345678901234567890';
  str_mfrlot text := '12345678901234567890123456789012345678901234567890';
  -- ^FX comment command
BEGIN
  str_content := $ZPL$^XA
^FO30,30
^GB1155,575,5^FS
^FO30,120
^GB1155,0,5^FS
^FO592.5,30
^GB0,90,5^FS
^FO30,210
^GB1155,0,5^FS
^FO30,300
^GB1155,0,5^FS
^FO592.5,210
^GB0,180,5^FS
^FO30,390
^GB1155,0,5^FS
^FO30,480
^GB1155,0,5^FS

^FO56,40^B3N,N,70,N^BY3,3^FD$ZPL$ ||- int_lot_no::text ||- $ZPL$^FS
^FO602,40^AQ^FDRA LOT #^FS
^FO602,80^AT^FD$ZPL$ ||- int_lot_no::text ||- $ZPL$^FS
^FO40,130^AQ^FDP/N^FS
^FO40,145^AV^FD$ZPL$ ||- str_pn ||- $ZPL$^FS
^FO40,220^AQ^FDQTY^FS
^FO40,260^AT^FD$ZPL$ ||- int_qty::text ||- $ZPL$^FS
^FO602,220^AQ^FDLOC^FS
^FO602,260^AT^FD$ZPL$ ||- str_location ||- $ZPL$^FS
^FO40,310^AQ^FDPO^FS
^FO40,350^AT^FD$ZPL$ ||- str_po_no ||- $ZPL$^FS
^FO602,310^AQ^FDPO DATE^FS
^FO602,350^AT^FD$ZPL$ ||- to_char(dat_po_received, 'MM/DD/YYYY') ||- $ZPL$^FS
^FO40,400^AQ^FDMFR^FS
^FO40,440^AT^FD$ZPL$ ||- str_mfrname ||- $ZPL$^FS
^FO40,490^AQ^FDMFR LOT #^FS
^FO40,530^AT^FD$ZPL$ ||- str_mfrlot ||- $ZPL$^FS
^PQ1
^XZ$ZPL$;

  RETURN str_content;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.sample_report() OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.sample_report() TO postgres;
GRANT EXECUTE ON FUNCTION zbr.sample_report() TO public;

--SELECT zbr.sample_report();


-- DROP FUNCTION zbr.letter_width(int_width integer, str_letter text);

CREATE OR REPLACE FUNCTION zbr.letter_width(int_width integer, str_letter text)
  RETURNS integer AS
$BODY$
DECLARE
  num_width numeric;

BEGIN
  CASE upper(str_letter)
  WHEN 'I' THEN num_width := 0.25;
  WHEN 'E', 'F', 'L' THEN num_width := 0.42;
  WHEN 'P', 'T' THEN num_width := 0.478;
  WHEN 'C', 'H', 'J', 'Q' THEN num_width := 0.507;
  WHEN 'B', 'D', 'K', 'R', 'V', 'X', 'Y' THEN num_width := 0.535;
  WHEN 'A', 'G', 'N', 'O', 'U', 'S' THEN num_width := 0.563;
  WHEN 'Z' THEN num_width := 0.591;
  WHEN 'M', '-' THEN num_width := 0.7;
  WHEN 'W' THEN num_width := 0.73;
  ELSE num_width := 0.563;
  END CASE;
  RETURN (num_width * int_width)::integer;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.letter_width(int_width integer, str_letter text) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.letter_width(int_width integer, str_letter text) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.letter_width(int_width integer, str_letter text) TO public;

--SELECT zbr.letter_width(int_width integer, str_letter text);


-- DROP FUNCTION zbr.largest_possible_font(str_value text, int_label_dot_width integer);

CREATE OR REPLACE FUNCTION zbr.largest_possible_font(str_value text, int_label_dot_width integer)
  RETURNS text AS
$BODY$
DECLARE
  str_working text := str_value;
  int_count integer := int_label_dot_width + 1;
  str_letter text;
  int_font integer := 87;

BEGIN
  WHILE int_count > int_label_dot_width AND int_font >= 80 LOOP
    int_count := 0;
    int_font := int_font - 1;
    WHILE length(str_working) > 0 LOOP
      str_letter := substring(str_working FROM 1 FOR 1);
      str_working := substring(str_working FROM 2);
      int_count := int_count + zbr.v_letter_width(chr(int_font), str_letter);
    END LOOP;
  END LOOP;
  RETURN chr(int_font);
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.largest_possible_font(str_value text, int_label_dot_width integer) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.largest_possible_font(str_value text, int_label_dot_width integer) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.largest_possible_font(str_value text, int_label_dot_width integer) TO public;

--SELECT zbr.largest_possible_font(str_value text, int_label_dot_width integer);


-- DROP FUNCTION zbr.font_width(str_font text);

CREATE OR REPLACE FUNCTION zbr.font_width(str_font text)
  RETURNS integer AS
$BODY$
DECLARE

BEGIN
  RETURN CASE str_font
	 WHEN 'A' THEN 6  -- 5 letter, 1 intercharacter gap
	 WHEN 'B' THEN 9  -- 7, 2
	 WHEN 'C' THEN 12 -- 10, 2
	 WHEN 'D' THEN 12 -- 10, 2
	 WHEN 'E' THEN 20 -- 15, 5
	 WHEN 'F' THEN 16 -- 13, 3
	 WHEN 'G' THEN 48 -- 40, 8
	 WHEN 'H' THEN 19 -- 13, 6
         WHEN 'P' THEN 18
         WHEN 'Q' THEN 24
         WHEN 'R' THEN 31
         WHEN 'S' THEN 35
         WHEN 'T' THEN 42
         WHEN 'U' THEN 53
         WHEN 'V' THEN 71
         END;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.font_width(str_font text) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.font_width(str_font text) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.font_width(str_font text) TO public;

--SELECT zbr.font_width(str_font text);


-- DROP FUNCTION zbr.center_font(str_value text, str_font text, int_label_dot_width integer);

CREATE OR REPLACE FUNCTION zbr.center_font(str_value text, str_font text, int_label_dot_width integer)
  RETURNS integer AS
$BODY$
DECLARE
  str_working text := str_value;
  int_count integer := 0;
  str_letter text;
  int_home integer;

BEGIN
  WHILE length(str_working) > 0 LOOP
    str_letter := substring(str_working FROM 1 FOR 1);
    str_working := substring(str_working FROM 2);
    int_count := int_count + zbr.v_letter_width(str_font, str_letter);
  END LOOP;
  int_home := (int_label_dot_width - int_count) / 2;
  IF int_home < 0 THEN
    RAISE NOTICE 'Warning. Text % is % wide, label is % wide. We''ll print as mext as we can.', quote_literal(str_value), int_count, int_label_dot_width;
  END IF;
  RETURN int_home;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.center_font(str_value text, str_font text, int_label_dot_width integer) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.center_font(str_value text, str_font text, int_label_dot_width integer) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.center_font(str_value text, str_font text, int_label_dot_width integer) TO public;

--SELECT zbr.center_font(str_value text, str_font text, int_label_dot_width integer);


-- DROP FUNCTION zbr.center_font(str_value text, int_height integer, int_width integer, int_label_dot_width integer);

CREATE OR REPLACE FUNCTION zbr.center_font(str_value text, int_height integer, int_width integer, int_label_dot_width integer)
  RETURNS integer AS
$BODY$
DECLARE
  str_working text := str_value;
  int_count integer := 0;
  str_letter text;
  int_home integer;

BEGIN
  WHILE length(str_working) > 0 LOOP
    str_letter := substring(str_working FROM 1 FOR 1);
    str_working := substring(str_working FROM 2);
    int_count := int_count + zbr.letter_width(int_width, str_letter);
  END LOOP;
  int_home := (int_label_dot_width - int_count) / 2;
  IF int_home < 0 THEN
    RAISE NOTICE 'Warning. Text % is % wide, label is % wide. We''ll print as much as we can.', quote_literal(str_value), int_count, int_label_dot_width;
  END IF;
  RETURN int_home;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.center_font(str_value text, int_height integer, int_width integer, int_label_dot_width integer) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.center_font(str_value text, int_height integer, int_width integer, int_label_dot_width integer) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.center_font(str_value text, int_height integer, int_width integer, int_label_dot_width integer) TO public;

--SELECT zbr.center_font(str_value text, int_height integer, int_width integer, int_label_dot_width integer);


-- DROP FUNCTION zbr.center_fit_font(int_left integer, int_top integer, str_value text, str_font text, int_label_dot_width integer);

CREATE OR REPLACE FUNCTION zbr.center_fit_font(int_left integer, int_top integer, str_value text, str_font text, int_label_dot_width integer)
  RETURNS text AS
$BODY$
DECLARE
  str_working text;
  int_count integer;
  str_letter text;
  int_home integer;
  bol_fit boolean;

BEGIN
  int_count := 0;
  bol_fit := true;
  str_working := str_value;

  WHILE bol_fit LOOP
    WHILE length(str_working) > 0 LOOP
      str_letter := substring(str_working FROM 1 FOR 1);
      str_working := substring(str_working FROM 2);
      int_count := int_count + zbr.v_letter_width(str_font, str_letter);
      RAISE NOTICE 'int_count: %', int_count::text;
    END LOOP;
    bol_fit := (int_label_dot_width < int_count);
    IF bol_fit THEN
      str_font := 'A';
      int_count := 0;
      str_working := str_value;
    END IF;
  END LOOP;

  int_home := (int_label_dot_width - int_count) / 2;

  RETURN '^FO' ||- (int_left + int_home)::text ||- ',' ||- int_top::text ||- '^A' ||- str_font ||- '^FD' ||- str_value ||- '^FS'
	|| '  length:' ||- int_count::text ||- '  home:' ||- int_home::text;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.center_fit_font(int_left integer, int_top integer, str_value text, str_font text, int_label_dot_width integer) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.center_fit_font(int_left integer, int_top integer, str_value text, str_font text, int_label_dot_width integer) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.center_fit_font(int_left integer, int_top integer, str_value text, str_font text, int_label_dot_width integer) TO public;

--SELECT zbr.center_fit_font(int_left integer, int_top integer, str_value text, str_font text, int_label_dot_width integer);


-- DROP FUNCTION zbr.center39(str_value text, int_label_dot_width integer, int_narrow_dots integer, num_ratio numeric);

CREATE OR REPLACE FUNCTION zbr.center39(str_value text, int_label_dot_width integer, int_narrow_dots integer, num_ratio numeric)
  RETURNS integer AS
$BODY$
DECLARE
  num_char_dots numeric := ((3 * int_narrow_dots * num_ratio) + (6 * int_narrow_dots)) * 1.074;
  int_home integer := ((int_label_dot_width - (num_char_dots * length('*' ||- str_value ||- '*'))) / 2)::integer;

BEGIN
  --this function centers 3 of 9 barcodes
  -- each char in 3 of 9 has nine wide modules and six narrow

  --RAISE NOTICE 'len:%, num_ratio:%', num_char_dots * length('*' ||- str_value ||- '*'), num_ratio;
  IF int_home < 0 THEN
    RAISE EXCEPTION 'Error. Barcode too wide for label at this width and ratio';
  END IF;
  RETURN int_home;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION zbr.center39(str_value text, int_label_dot_width integer, int_narrow_dots integer, num_ratio numeric) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.center39(str_value text, int_label_dot_width integer, int_narrow_dots integer, num_ratio numeric) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.center39(str_value text, int_label_dot_width integer, int_narrow_dots integer, num_ratio numeric) TO public;

--SELECT zbr.center39(str_value text, int_label_dot_width integer, int_narrow_dots integer, num_ratio numeric);


-- DROP FUNCTION zbr.l8(integer, integer);

CREATE OR REPLACE FUNCTION zbr.l8(integer, integer)
  RETURNS text AS
$BODY$
  SELECT ($1/800.0 * $2)::integer::text;
$BODY$
  LANGUAGE sql VOLATILE
  COST 100;

ALTER FUNCTION zbr.l8(integer, integer) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.l8(integer, integer) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.l8(integer, integer) TO public;

--SELECT zbr.l8(integer, integer);


-- DROP FUNCTION zbr.l6(integer, integer);

CREATE OR REPLACE FUNCTION zbr.l6(integer, integer)
  RETURNS text AS
$BODY$
  SELECT ($1/600.0 * $2)::integer::text;
$BODY$
  LANGUAGE sql VOLATILE
  COST 100;

ALTER FUNCTION zbr.l6(integer, integer) OWNER TO postgres;
GRANT EXECUTE ON FUNCTION zbr.l6(integer, integer) TO postgres;
GRANT EXECUTE ON FUNCTION zbr.l6(integer, integer) TO public;

--SELECT zbr.l6(integer, integer);
