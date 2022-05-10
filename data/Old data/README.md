# File naming code

Template: ```RSCRBZ_DDmonthYYYY_FID```

```{c}
recording_mode = {
	.R = { 
		T = test,
		R = actual_recording,
	}
}
```

```{c}
sampling_frequency = {
	.S = { 
		0 = 16kHz, 
		1 = 32kHz,
		2 = 44.1kHz, 
		3 = 96kHz, 
		4 = 112kHz,
		5 = 128kHz,
		6 = 144kHz,
		7 = 160kHz,
		8 = 176kHz,
		9 = 192kHz,
	}
}
```

```{c}
sampling_frequency = {
	.C = { 
		M = mono,
		S = stereo,
	}
}
```

```{c}
resolution_bits = {
	.RB = { 
		16 = 16_bits,
	}
}
```

```{c}
day_of_recording = {
	.DD = { 
		two_digits_day,
	}
}
```

```{c}
month_of_recording = {
	.month = { 
		jan,
		feb,
		mar,
		apr,
		may,
		jun,
		jul,
		ago,
		sep,
		oct,
		nov,
		dec,
	}
}
```

```{c}
year_of_recording = {
	.YYYY = { 
		four_digits_year,
	}
}
```

```{c}
file_unique_id = {
	.FID = { 
		000 = 3_number_digit,
	}
}
```
