DO $$
DECLARE
    d date := '2024-01-01';
    id_counter bigint := 1;
    usernames text := 'YourName';
    methods text[] := ARRAY['Cash','UPI','Card','Bank Transfer'];
    categories_expense text[] := ARRAY['Groceries','Food','Transport','Entertainment','Shopping','Healthcare','Utilities','Miscellaneous','Housing'];
    categories_income text[] := ARRAY['Salary','Freelance','Gift','Investment'];
    amount numeric;
    category text;
    method text;
    cashflow text;
BEGIN
    WHILE d <= '2024-12-31' LOOP

        -- Recurring monthly rent on the 1st
        IF extract(day FROM d) = 1 THEN
            INSERT INTO public.payments(id, username, amount, label, date, method, category, cashflow)
            VALUES(
                id_counter,
                usernames,
                1200.00,
                'Monthly Rent',
                d,
                methods[1]::public.method, -- CAST to enum
                'Housing',
                'Expense'::public.transactiontype -- CAST to enum
            );
            id_counter := id_counter + 1;
        END IF;

        -- Recurring monthly salary on the 5th
        IF extract(day FROM d) = 5 THEN
            INSERT INTO public.payments(id, username, amount, label, date, method, category, cashflow)
            VALUES(
                id_counter,
                usernames,
                3200 + (random()*300)::numeric,
                'Salary Credit',
                d,
                'Bank Transfer'::public.method,
                'Salary',
                'Income'::public.transactiontype
            );
            id_counter := id_counter + 1;
        END IF;

        -- Random daily expenses (50% chance)
        IF random() < 0.5 THEN
            FOR i IN 1..(1 + floor(random()*2)) LOOP
                amount := (CASE WHEN random()<0.1 THEN 200 + random()*500 ELSE 10 + random()*100 END)::numeric(10,2);
                category := categories_expense[1 + floor(random()*(array_length(categories_expense,1)))];
                method := methods[1 + floor(random()*(array_length(methods,1)))];
                cashflow := 'Expense';

                INSERT INTO public.payments(id, username, amount, label, date, method, category, cashflow)
                VALUES(
                    id_counter,
                    usernames,
                    amount,
                    category,
                    d,
                    method::public.method,  -- CAST to enum
                    category,
                    cashflow::public.transactiontype -- CAST to enum
                );
                id_counter := id_counter + 1;
            END LOOP;
        END IF;

        -- Occasional income (5% chance)
        IF random() < 0.05 THEN
            amount := (500 + random()*3000)::numeric(10,2);
            category := categories_income[1 + floor(random()*(array_length(categories_income,1)))];
            method := methods[1 + floor(random()*(array_length(methods,1)))];
            cashflow := 'Income';

            INSERT INTO public.payments(id, username, amount, label, date, method, category, cashflow)
            VALUES(
                id_counter,
                usernames,
                amount,
                category,
                d,
                method::public.method,  -- CAST to enum
                category,
                cashflow::public.transactiontype -- CAST to enum
            );
            id_counter := id_counter + 1;
        END IF;

        d := d + interval '1 day';
    END LOOP;
END $$;
