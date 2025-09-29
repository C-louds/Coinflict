--
-- PostgreSQL database dump
--

\restrict c5yNnRrxxrDehXJBRfBIBGZDtgfQnqjczS1JELqlQi8dh9GjfuEApRyijVSxVxx

-- Dumped from database version 17.6 (Ubuntu 17.6-1.pgdg24.04+1)
-- Dumped by pg_dump version 17.6 (Ubuntu 17.6-1.pgdg24.04+1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: method; Type: TYPE; Schema: public; Owner: clouds
--

CREATE TYPE public.method AS ENUM (
    'Cash',
    'UPI',
    'Card',
    'Bank Transfer'
);


ALTER TYPE public.method OWNER TO clouds;

--
-- Name: transactiontype; Type: TYPE; Schema: public; Owner: clouds
--

CREATE TYPE public.transactiontype AS ENUM (
    'Expense',
    'Income'
);


ALTER TYPE public.transactiontype OWNER TO clouds;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: payments; Type: TABLE; Schema: public; Owner: clouds
--

CREATE TABLE public.payments (
    id bigint NOT NULL,
    username character varying(20) DEFAULT 'Dattraj'::character varying NOT NULL,
    amount numeric(10,2) NOT NULL,
    label character varying(50) DEFAULT 'IDK'::character varying NOT NULL,
    date date DEFAULT CURRENT_DATE,
    method public.method DEFAULT 'Cash'::public.method NOT NULL,
    category character varying(15) DEFAULT 'Miscellaneous'::character varying NOT NULL,
    cashflow public.transactiontype NOT NULL
);


ALTER TABLE public.payments OWNER TO clouds;

--
-- Name: payments_id_seq; Type: SEQUENCE; Schema: public; Owner: clouds
--

CREATE SEQUENCE public.payments_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.payments_id_seq OWNER TO clouds;

--
-- Name: payments_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: clouds
--

ALTER SEQUENCE public.payments_id_seq OWNED BY public.payments.id;


--
-- Name: payments id; Type: DEFAULT; Schema: public; Owner: clouds
--

ALTER TABLE ONLY public.payments ALTER COLUMN id SET DEFAULT nextval('public.payments_id_seq'::regclass);


--
-- Name: payments payments_pkey; Type: CONSTRAINT; Schema: public; Owner: clouds
--

ALTER TABLE ONLY public.payments
    ADD CONSTRAINT payments_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

\unrestrict c5yNnRrxxrDehXJBRfBIBGZDtgfQnqjczS1JELqlQi8dh9GjfuEApRyijVSxVxx

